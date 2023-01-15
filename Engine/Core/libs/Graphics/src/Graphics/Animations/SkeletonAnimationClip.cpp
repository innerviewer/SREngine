//
// Created by Igor on 07/12/2022.
//

#include <Graphics/Animations/SkeletonAnimationClip.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace SR_ANIMATIONS_NS {
    const AnimationPose& SkeletonAnimationClip::GetCurrentPose() const {
        m_currentPose.bonesLocalPoses[0] = GetBoneLocalPose(0, m_currentTime);

        auto bonesCount = 0;//static_cast<uint8_t>(m_skeleton->bones.size());

        for (uint8_t boneIndex = 1; boneIndex < bonesCount; boneIndex++) {
            m_currentPose.bonesLocalPoses[boneIndex] = GetBoneLocalPose(boneIndex, m_currentTime);
        }

        return m_currentPose;
    }

    void SkeletonAnimationClip::Load(SR_UTILS_NS::Path path) {
        Assimp::Importer* importer = new Assimp::Importer();
        const aiScene* scene = importer->ReadFile(path.ToString(), aiProcess_Triangulate);

        double duration = scene->mAnimations[0]->mDuration;
        double rate = scene->mAnimations[0]->mTicksPerSecond;
        std::vector<BoneAnimationChannel> bonesAnimationChannels;


        delete scene;
        delete importer;
    }

    void SkeletonAnimationClip::IncreaseCurrentTime(float delta) {
        m_currentTime += delta * m_rate;

        if (m_currentTime > m_duration) {
            int overflowParts = static_cast<int>(m_currentTime / m_duration);
            m_currentTime -= m_duration * static_cast<float>(overflowParts);
        }
    }

    BonePose SkeletonAnimationClip::GetBoneLocalPose(uint8_t boneIndex, float time) const {
        {
            const std::vector<BoneAnimationPositionFrame>& positionFrames =
                    m_bonesAnimationChannels[boneIndex].positionFrames;

            auto position = GetMixedAdjacentFrames<glm::vec3, BoneAnimationPositionFrame>(positionFrames, time);

            const std::vector<BoneAnimationOrientationFrame>& orientationFrames =
                    m_bonesAnimationChannels[boneIndex].orientationFrames;

            auto orientation = GetMixedAdjacentFrames<glm::quat, BoneAnimationOrientationFrame>(orientationFrames, time);

            return BonePose(position, orientation, 0.0f); ///TODO: scale пока-что не используется, но должен передаваться, потому в будущем реализовать его передачу
        }
    }

    template<class T, class S>
    T SkeletonAnimationClip::GetMixedAdjacentFrames(const std::vector<S>& frames, float time) const
    {
        S tempFrame;
        tempFrame.time = time;

        auto frameIt = std::upper_bound(frames.begin(), frames.end(),
                                        tempFrame, [](const S& a, const S& b) {
                    return a.time < b.time;
                });

        if (frameIt == frames.end()) {
            return (frames.size() > 0) ? GetKeyframeValue<T, S>(*std::prev(frames.end())) : GetIdentity<T>();
        }
        else {
            T next = GetKeyframeValue<T, S>(*frameIt);
            T prev = (frameIt == frames.begin()) ? GetIdentity<T>() : GetKeyframeValue<T, S>(*std::prev(frameIt));

            float currentFrameTime = frameIt->time;
            float prevFrameTime = (frameIt == frames.begin()) ? 0 : std::prev(frameIt)->time;

            float framesTimeDelta = currentFrameTime - prevFrameTime;

            return GetInterpolatedValue<T>(prev, next, (time - prevFrameTime) / framesTimeDelta);
        }
    }
    template<>
    glm::vec3 SkeletonAnimationClip::GetIdentity() const
    {
        return glm::vec3(0.0f);
    }

    template<>
    glm::quat SkeletonAnimationClip::GetIdentity() const
    {
        return glm::identity<glm::quat>();
    }

    template<>
    glm::vec3 SkeletonAnimationClip::GetKeyframeValue(const BoneAnimationPositionFrame& frame) const
    {
        return frame.position;
    }

    template<>
    glm::quat SkeletonAnimationClip::GetKeyframeValue(const BoneAnimationOrientationFrame& frame) const
    {
        return frame.orientation;
    }

    template<>
    glm::vec3 SkeletonAnimationClip::GetInterpolatedValue(const glm::vec3& first, const glm::vec3& second, float delta) const
    {
        return glm::mix(first, second, delta);
    }

    template<>
    glm::quat SkeletonAnimationClip::GetInterpolatedValue(const glm::quat& first, const glm::quat& second, float delta) const
    {
        return glm::slerp(first, second, delta);
    }
}
