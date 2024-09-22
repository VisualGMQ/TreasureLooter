#include "pch.hpp"
#include "context.hpp"

int main(int argc, char** argv) {
    tl::Context::Init();
    tl::Context::GetInst().Update();
    tl::Context::Destroy();

    return 0;
}
