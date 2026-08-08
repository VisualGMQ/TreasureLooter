#pragma once
#include "tool_context.hpp"

class TilemapDetourDataBakerContext: public  ToolContext {
public:
    static void Init();
    static TilemapDetourDataBakerContext& GetInst();
    static void Destroy();

    void Initialize(int argc, char** argv) override;

private:
    TilemapHandle m_tilemap;
    std::vector<char> m_layer_selected;

    TilemapDetourDataBakerContext() = default;

protected:
    void update() override;
    TilemapDetourDataHandle build();
};
