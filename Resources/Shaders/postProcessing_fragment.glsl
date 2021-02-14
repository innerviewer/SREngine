#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D scene;
uniform sampler2D bloomBlur;
uniform sampler2D skybox;
uniform sampler2D stencil;
uniform sampler2D depth;

uniform float exposure;
uniform float gamma;

//uniform vec3 ColorCorrection;

float LinearizeDepth(float depth) {
    // преобразуем обратно в NDC
    float z = depth * 2.0 - 1.0;
    return (2.0 * 0.1 * 100.0) / (100.0 + 0.1 - z * (100.0 - 0.1));
}

void DrawStencil() {
    // outline thickness
    int w = 3;

    // if the pixel is black (we are on the silhouette)
    if (texture(stencil, TexCoords).a == 0) {
        vec2 size = 1.0f / textureSize(stencil, 0);

        for (int i = -w; i <= +w; i++)
            for (int j = -w; j <= +w; j++) {
                if (i == 0 && j == 0)
                 continue;

                vec2 offset = vec2(i, j) * size;

                // and if one of the pixel-neighbor is white (we are on the border)
                if (texture(stencil, TexCoords + offset).a == 1) {
                    //FragColor = vec4(vec3(1.0f), 1.0f);
                    FragColor = vec4(vec3(1, 1, 1), 1.0f);
                    return;
                }
            }
    }
}

void DrawStencilFast() {
    // and if one of the pixel-neighbor is white (we are on the border)
    if (texture(stencil, TexCoords).a == 1) {
        int w = 3;

        vec2 size = 1.0f / textureSize(stencil, 0);

        for (int i = -w; i <= +w; i++) {
            for (int j = -w; j <= +w; j++) {
                if (i == 0 && j == 0)
                    continue;

                if (texture(stencil, TexCoords + vec2(i, j) * size).a == 0) {
                    if (LinearizeDepth(gl_FragCoord.z) / 100 > texture(depth, TexCoords + vec2(i, j) * size).r)
                        FragColor = vec4(vec3(1, 1, 1), 1.0f);
                    else
                        FragColor = vec4(vec3(1, 0, 0), 1.0f);
                }
            }
        }

        return;
    }
}

void main() {
    //vec3 hdrColor = texture(scene, TexCoords).rgb;
    //hdrColor += texture(bloomBlur, TexCoords).rgb; // additive blending

    //float alpha = texture(bloomBlur, TexCoords).a;
    //alpha += texture(scene, TexCoords).a;
    //if (alpha > 1)
    //    alpha = 1;

    vec3 sceneColor  = texture(scene,     TexCoords).rgb;
    vec3 skyboxColor = texture(skybox,    TexCoords).rgb;
    vec3 bloomColor  = texture(bloomBlur, TexCoords).rgb;

    vec3 hdrColor;

    if (sceneColor == vec3(0,0,0)) {
        hdrColor = skyboxColor;
    } else {
        hdrColor = sceneColor;
        float alpha = texture(scene, TexCoords).a;
        if (alpha != 1)
            hdrColor = (hdrColor * alpha) + skyboxColor;
    }

    hdrColor += texture(bloomBlur, TexCoords).rgb;

    // tone mapping
    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);

    //result.r *= ColorCorrection.r;
    //result.g *= ColorCorrection.g;
    //result.b *= ColorCorrection.b;

    // also gamma correct while we're at it
    result = pow(result, vec3(1.0 / gamma));
    FragColor = vec4(result, 1);

    DrawStencilFast();
}