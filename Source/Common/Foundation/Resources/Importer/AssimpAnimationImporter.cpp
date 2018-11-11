
#pragma warning ( push, 0 )
#include <assimp/Importer.hpp>     
#include <assimp/scene.h>          
#include <assimp/postprocess.h>  
#pragma warning ( pop )

#include <Foundation/Memory/AlignedPtr.h>

#include "AssimpAnimationImporter.h"

namespace MoonGlare::Resources::Importer {

void ImportAssimpAnimation(const aiScene *scene, int animIndex, pugi::xml_node animSetXml, AnimationImport &output) {
    assert(scene);
    assert(animIndex >= 0 && animIndex < scene->mNumAnimations);

    const auto *assimpAnim = scene->mAnimations[animIndex];
    pugi::xml_node animSetNode = animSetXml.find_child_by_attribute("AnimationSet", "Index", std::to_string(animIndex).c_str());

    //calculate memory requirement:
    size_t AnimationDataSize = 0;
    size_t stringArraySize = 1; //first byte to be zero  
    stringArraySize += assimpAnim->mName.length + 1;

    for (size_t ch = 0; ch < assimpAnim->mNumChannels; ++ch) {
        size_t chsize = 0;
        auto &channel = assimpAnim->mChannels[ch];
        chsize += Memory::Align16(sizeof(PositionKey) * channel->mNumPositionKeys);
        chsize += Memory::Align16(sizeof(ScalingKey)  * channel->mNumScalingKeys);
        chsize += Memory::Align16(sizeof(RotationKey) * channel->mNumRotationKeys);
        stringArraySize += channel->mNodeName.length + 1;
        AnimationDataSize += chsize;
    }

    if (animSetNode) {
        for (auto node = animSetNode.first_child(); node; node = node.next_sibling()) {
            std::string n = node.attribute("Name").as_string("");
            stringArraySize += n.size() + 1;
        }
    }

    size_t dataAllocOffset = Memory::Align16(stringArraySize);
    AnimationDataSize += dataAllocOffset;

    output.memory.reset(new char[AnimationDataSize]);
    char *mem = output.memory.get();
    memset(mem, 0, AnimationDataSize);
    SkeletalAnimation &animInfo = output.animation;
    animInfo = {};

    animInfo.stringArrayBase = (char*)mem;

    size_t stringAllocOffset = 1;

    auto pushString = [&stringAllocOffset, &animInfo](const char* text) {
        size_t len = strlen(text);
        size_t offset = stringAllocOffset;
        stringAllocOffset += len + 1;
        char *out = (char*)animInfo.stringArrayBase + offset;
        strcpy(out, text);
        out[len] = '\0';
        return offset;
    };

    auto allocData = [&dataAllocOffset, mem](auto arr, size_t cnt) -> decltype(arr) {
        size_t elemSize = sizeof(arr[0]);
        size_t len = cnt * elemSize;
        len = Memory::Align16(len);
        size_t offset = dataAllocOffset;
        dataAllocOffset = Memory::Align16(dataAllocOffset + len);
        assert((dataAllocOffset & 0xF) == 0);
        return reinterpret_cast<decltype(arr)>(mem + offset);
    };

    animInfo.animationNameOffset = pushString(assimpAnim->mName.data);

    if (animSetNode) {
        size_t index = 0;
        for (auto node = animSetNode.first_child(); node; node = node.next_sibling()) {
            animInfo.animationSetNameOffset[index] = pushString(node.attribute("Name").as_string(""));
            auto start = node.attribute("Start").as_uint();
            auto end = node.attribute("End").as_uint();
            auto loop = node.attribute("Loop").as_bool();

            auto &set = animInfo.animationSet[index];
            set.firstFrame = start;
            set.endFrame = end;
            set.loop = loop ? 1 : 0;
            ++index;
        }
        animInfo.animationSetCount = index;
    }

    animInfo.channelCount = assimpAnim->mNumChannels;
    for (size_t ch = 0; ch < assimpAnim->mNumChannels; ++ch) {
        auto &assimpChannel = assimpAnim->mChannels[ch];
        animInfo.channelNameOffset[ch] = pushString(assimpChannel->mNodeName.data);
        auto &channel = animInfo.channel[ch];

        channel.positionKeyCount = assimpChannel->mNumPositionKeys;
        channel.rotationKeyCount = assimpChannel->mNumRotationKeys;
        channel.scalingKeyCount = assimpChannel->mNumScalingKeys;

        channel.positionKey = allocData(channel.positionKey, channel.positionKeyCount);
        for (size_t key = 0; key < channel.positionKeyCount; ++key) {
            auto &k = channel.positionKey[key];
            const auto &v = assimpChannel->mPositionKeys[key].mValue;
            k.time = assimpChannel->mPositionKeys[key].mTime;
            k.value = { v.x, v.y, v.z };
        }

        channel.rotationKey = allocData(channel.rotationKey, channel.rotationKeyCount);
        for (size_t key = 0; key < channel.rotationKeyCount; ++key) {
            auto &k = channel.rotationKey[key];
            const auto &v = assimpChannel->mRotationKeys[key].mValue;
            k.time = assimpChannel->mRotationKeys[key].mTime;
            if constexpr (std::is_same_v<RotationKey::item_t, emath::Quaternion>) {
                k.value = { v.w, v.x, v.y, v.z };
            } else {
                if constexpr (std::is_same_v<RotationKey::item_t, glm::fvec3>) {
                    k.value = { v.x, v.y, v.z, v.w };
                } else {
                    __debugbreak();
                }
            }
        }

        channel.scalingKey = allocData(channel.scalingKey, channel.scalingKeyCount);
        for (size_t key = 0; key < channel.scalingKeyCount; ++key) {
            auto &k = channel.scalingKey[key];
            const auto &v = assimpChannel->mScalingKeys[key].mValue;
            k.time = assimpChannel->mScalingKeys[key].mTime;
            k.value = { v.x, v.y, v.z };
        }
    }

    assert(stringAllocOffset == stringArraySize);
    assert(AnimationDataSize == dataAllocOffset);

    animInfo.duration = assimpAnim->mDuration;
    animInfo.ticksPerSecond = assimpAnim->mTicksPerSecond;

    animInfo.memoryBlockFront = mem;
    animInfo.memoryBlockSize = AnimationDataSize;
}

}
