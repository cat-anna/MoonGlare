#pragma once

#include "../../Commands/CommandQueue.h"
#include "../../Commands/OpenGL/ArrayCommands.h"

namespace MoonGlare::Renderer::Resources::Builder {

struct VAOBuilder {
    using Conf = Configuration::VAO;

    using InputChannels = Conf::InputChannels;
    using ChannelType = Conf::ChannelType;

    void SelfTest() {
        assert(this);
        assert(m_Queue);
        assert(m_BuffersPtr);
        assert(m_HandlePtr);
    }

    void BindVAO(Commands::CommandKey key = {}) {
        SelfTest();
        m_Queue->MakeCommandKey<Commands::VAOBindResource>(key, m_HandlePtr);
    }
    void UnBindVAO(Commands::CommandKey key = {}) {
        SelfTest();
        m_Queue->MakeCommandKey<Commands::VAOBind>(key, Device::InvalidVAOHandle);
    }

    void BeginDataChange(Commands::CommandKey key = {}) {
        SelfTest();
        currentKey = key;
        BindVAO(key);
    }
    void EndDataChange() {
    }

    void CreateChannel(InputChannels iChannel) {
        auto Channel = static_cast<ChannelType>(iChannel);
        SelfTest();
        if (m_BuffersPtr[Channel] == Device::InvalidBufferHandle)
            m_Queue->MakeCommandKey<Commands::BufferSingleAllocate>(currentKey, &m_BuffersPtr[Channel]);
    }

    template <typename T, GLint ElementSize = 1>
    void SetChannelData(InputChannels iChannel, const T* data, size_t ElementCount, bool Dynamic = false, bool Normalized = false) {
        auto Channel = static_cast<ChannelType>(iChannel);
        SelfTest();
        m_Queue->MakeCommandKey<Commands::BindArrayBufferResource>(currentKey)->m_Handle = &m_BuffersPtr[Channel];

        GLsizeiptr bytecount = ElementCount * ElementSize * sizeof(T);
        if (Dynamic)
            m_Queue->MakeCommandKey<Commands::ArrayBufferDynamicData>(currentKey, bytecount, (const void*)data);
        else
            m_Queue->MakeCommandKey<Commands::ArrayBufferStaticData>(currentKey, bytecount, (const void*)data);

        if (Normalized)
            m_Queue->MakeCommandKey<Commands::ArrayBufferNormalizedChannel>(currentKey, Channel, ElementSize, Device::TypeId<T>);
        else
            m_Queue->MakeCommandKey<Commands::ArrayBufferChannel>(currentKey, Channel, ElementSize, Device::TypeId<T>);
    }

    template <typename T, size_t ElemCount>
    void SetIndex(InputChannels iChannel, const std::array<T, ElemCount> &table, bool Dynamic = false) {
        SetIndex(iChannel, &table[0], table.size(), Dynamic);
    }

    template <typename T>
    void SetIndex(InputChannels iChannel, const T* data, size_t ElementCount, bool Dynamic = false) {
        auto Channel = static_cast<ChannelType>(iChannel);
        SelfTest();
        m_Queue->MakeCommandKey<Commands::BindArrayIndexBufferResource>(currentKey)->m_Handle = &m_BuffersPtr[Channel];

        GLsizeiptr bytecount = ElementCount * sizeof(T);
        if (Dynamic)
            m_Queue->MakeCommandKey<Commands::ArrayIndexBufferDynamicData>(currentKey, bytecount, (const void*)data);
        else
            m_Queue->MakeCommandKey<Commands::ArrayIndexBufferStaticData>(currentKey, bytecount, (const void*)data);
    }


    Commands::CommandQueue *m_Queue;
    Device::BufferHandle *m_BuffersPtr;
    Device::VAOHandle* m_HandlePtr;
    Commands::CommandKey currentKey;
    uint16_t _padding;
};
static_assert(std::is_trivial<VAOBuilder>::value, "must be trivial!");
static_assert((sizeof(VAOBuilder) % 16) == 0, "Invalid size!");

} //namespace MoonGlare::Renderer::Resources::Builder 
