#include "level/test/test_maze.hpp"
#include "context.hpp"
#include "level/test/play_controller.hpp"

namespace tl {

constexpr size_t MazeWidth = 21;
constexpr size_t MazeHeight = 21;

// clang-format off
constexpr std::string_view Maze =
    "  ###################"
    "                    #"
    "##### # ####### # ###"
    "#   # # # #     # # #"
    "# # ##### ### ### # #"
    "# # #   #       #   #"
    "### # ##### # ##### #"
    "#     #     #   # # #"
    "### ### ####### # # #"
    "#   #   # #   # # # #"
    "# # # # # # ##### ###"
    "# # # # # #         #"
    "### # ### ### ##### #"
    "#   #         #     #"
    "# ### ### # # ##### #"
    "#     #   # # #     #"
    "# ########### # ### #"
    "#     #     # # #   #"
    "##### ##### # # ### #"
    "#           # # #    "
    "###################  ";
// clang-format on

void TestMazeLevel::Init() {
    auto& goMgr = Context::GetInst().sceneMgr->GetCurScene().GetGOMgr();
    GameObject* go = goMgr.Find("test/maze/rectPrefab");
    TL_RETURN_IF_FALSE(go);

    auto& curScene = Context::GetInst().sceneMgr->GetCurScene();

    GameObject* root = curScene.GetRootGO();

    for (size_t i = 0; i < MazeHeight; i++) {
        for (size_t j = 0; j < MazeWidth; j++) {
            const char c = Maze[i * MazeWidth + j];
            if (c == '#') {
                GameObject* newGO = goMgr.Clone(go->GetID());
                TL_CONTINUE_IF_FALSE(newGO);
                newGO->transform.position =
                    newGO->physicActor.shape.aabb.halfSize * 2.0;
                newGO->transform.position.x *= j;
                newGO->transform.position.y *= i;
                newGO->transform.position += Vec2{25, 25};
                root->AppendChild(*newGO);
            }
        }
    }
}


void TestMazeLevel::Enter() {
    Context::GetInst().debugMgr->enableDrawCollisionShapes = true;

    GameObject* go = Context::GetInst().sceneMgr->GetCurScene().GetGOMgr().Find(
        "test/maze/circlePrefab");
    TL_RETURN_IF_FALSE(go);
    Context::GetInst().gameController = std::make_unique<CommonMoveController>(
        go->GetID());
}
}
