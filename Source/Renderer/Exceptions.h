#pragma once

namespace MoonGlare::Renderer {

class RendererException : public std::runtime_error {
public:
    RendererException(std::string msg) : std::runtime_error(std::move(msg)) {}
};

class CriticalException : public RendererException {
public:
    CriticalException(std::string msg) : RendererException(std::move(msg)) {}
};

class InitFailureException : public CriticalException {
public:
    InitFailureException(std::string msg) : CriticalException(std::move(msg)) {}
};

} //namespace MoonGlare::Renderer
