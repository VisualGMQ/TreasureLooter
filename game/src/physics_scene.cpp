#include "physics_scene.hpp"

#include "context.hpp"
#include "gameobject.hpp"

namespace tl {
VectorSplitResult SplitVector(const Vec2& vel, const Vec2& norm) {
    Vec2 vertical = ProjectOn(vel, norm);
    Vec2 horizontal = vel - vertical;
    return {vertical, horizontal};
}

void PhysicsScene::MarkAsPhysics(GameObject* go) {
    if (go->physicActor) {
        go->physicActor.collideShape_ = GetShapeRelateBy(*go);
        MarkedActor actor;
        actor.actor = &go->physicActor;
        actor.go = go;
        actors_.push_back(actor);
    }
    if (go->tilemap) {
        for (auto& layer : go->tilemap->GetLayers()) {
            if (layer->GetType() == MapLayerType::Tiles) {
                auto tileLayer = layer->AsTileLayer();
                for (int i = 0; i < tileLayer->GetSize().w; i++) {
                    for (int j = 0; j < tileLayer->GetSize().h; j++) {
                        Tile* tile = tileLayer->GetTile(i, j);
                        TL_CONTINUE_IF_FALSE(
                            tile && tile->tilesetIndex && tile->actor);
                        bool isVFlip = tile->flip & Flip::Vertical;
                        bool isHFlip = tile->flip & Flip::Horizontal;
                        const TileSet& tileset =
                            go->tilemap->GetTileSet(tile->tilesetIndex.value());
                        Transform localTransform{tileset.tileSize * Vec2(i, j)};
                        Transform scaleTransform{
                            Vec2{isHFlip ? tile->region.size.w : 0,
                                 isVFlip ? tile->region.size.h : 0},
                            Vec2{isHFlip ? -1.0f : 1.0f,
                                 isVFlip ? -1.0f : 1.0f}
                        };
                        Transform tileLocalTransform = CalcTransformFromParent(
                            localTransform, scaleTransform);
                        Transform tileGlobalTransform = CalcTransformFromParent(
                            go->GetGlobalTransform(), tileLocalTransform);
                        tile->actor.collideShape_ =
                            GetShapeRelateBy(tileGlobalTransform, tile->actor);

                        MarkedActor actor;
                        actor.go = go;
                        actor.tile = tile;
                        actor.actor = &tile->actor;
                        actors_.push_back(actor);
                    }
                }
            }
        }
    }
}

void PhysicsScene::Update(TimeType delta) {
    constexpr int MaxIterCount = 16;
    int iterCount = MaxIterCount;

    generateContacts();

    while (iterCount > 0 && !hitInfos_.empty()) {
        sortContacts();
        handleContacts();
        generateContacts();
        iterCount--;
    }

    if (hitInfos_.empty()) {
        handleNoContactMove();
    }

    hitInfos_.clear();

    auto& eventMgr = Context::GetInst().eventMgr;

    for (int i = 0; i < actors_.size(); i++) {
        for (int j = i; j < actors_.size(); j++) {
            const MarkedActor* trigger = nullptr;
            const MarkedActor* solid = nullptr;

            MarkedActor& actor1 = actors_[i];
            MarkedActor& actor2 = actors_[j];
            if (actor1.actor->isTrigger && !actor2.actor->isTrigger) {
                trigger = &actor1;
                solid = &actor2;
            } else if (!actor1.actor->isTrigger && actor2.actor->isTrigger) {
                trigger = &actor1;
                solid = &actor2;
            } else {
                continue;
            }

            auto it = std::find_if(trigger->actor->enteredGOList_.begin(),
                         trigger->actor->enteredGOList_.end(),
                         [goid = solid->go->GetID()](const GameObjectID& id) {
                             return id == goid;
                         });
            bool isInArea = trigger->actor->enteredGOList_.end() != it;
            bool isOverlap = checkOverlap(*trigger->actor, *solid->actor);
            if (isInArea && !isOverlap) {
                trigger->actor->enteredGOList_.erase(it);
                eventMgr->EnqueueEnterTriggerAreaEvent(solid->go, *trigger);
            } else if (!isInArea && isOverlap) {
                trigger->actor->enteredGOList_.push_back(solid->go->GetID());
                eventMgr->EnqueueEnterTriggerAreaEvent(trigger->go, *solid);
            }
        }
    }
}

void PhysicsScene::ClearActors() {
    actors_.clear();
}

void PhysicsScene::handleNoContactMove() {
    for (auto actor : actors_) {
        actor.actor->collideShape_.SetCenter(
            actor.actor->collideShape_.GetCenter() +
            actor.actor->movement_);
        actor.actor->movement_ = Vec2::ZERO;
    }
}

void PhysicsScene::generateContacts() {
    for (size_t i = 0; i < actors_.size(); i++) {
        std::optional<SweepHitInfo> minHitInfo;
        PhysicActor* actor1 = actors_[i].actor;
        for (size_t j = i + 1; j < actors_.size(); j++) {
            PhysicActor* actor2 = actors_[j].actor;

            TL_CONTINUE_IF_FALSE(actor1->filter & actor2->filter);
            TL_CONTINUE_IF_FALSE(!actor1->isTrigger || !actor2->isTrigger);

            SweepHitInfo hit = sweep(*actor1, *actor2);
            hit.src = actor1;
            hit.dst = actor2;

            TL_CONTINUE_IF_FALSE(hit && hit.t <= 1);

            if (!minHitInfo || hit.t < minHitInfo->t) {
                minHitInfo = hit;
            }
        }

        if (minHitInfo) {
            hitInfos_.push_back(minHitInfo.value());
        }
    }
}

void PhysicsScene::sortContacts() {
    std::stable_sort(std::begin(hitInfos_), std::end(hitInfos_),
                     [](const SweepHitInfo& h1, const SweepHitInfo& h2) {
                         return h1.t < h2.t;
                     });
}

void PhysicsScene::handleContacts() {
    SweepHitInfo hit = hitInfos_[0];
    moveAndSlide(hit, 0.01);
    hitInfos_.clear();
}

void PhysicsScene::moveAndSlide(SweepHitInfo& hit, float Threshould) {
    float t = std::max(hit.t - Threshould, 0.0f);
    do {
        Vec2 dir = hit.src->movement_;
        float len = dir.Length();
        TL_BREAK_IF_FALSE(len > 0);

        auto [vertical, horizontal] = SplitVector(dir * (1.0 - t), hit.normal);

        hit.src->collideShape_.SetCenter(hit.src->collideShape_.GetCenter() +
                                         t * dir);
        hit.src->movement_ = Vec2::ZERO;

        if (horizontal.LengthSqrd() > Threshould) {
            hit.src->movement_ = horizontal;
        }
    } while (0);

    do {
        Vec2 dir = hit.dst->movement_;
        float len = dir.Length();
        TL_BREAK_IF_FALSE(len > 0);

        auto [vertical, horizontal] = SplitVector(dir * (1.0 - t), -hit.normal);

        hit.dst->collideShape_.SetCenter(hit.dst->collideShape_.GetCenter() +
                                         t * dir);
        hit.dst->movement_ = Vec2::ZERO;

        if (horizontal.LengthSqrd() > Threshould) {
            hit.dst->movement_ = horizontal;
        }
    } while (0);
}

bool PhysicsScene::quickCheckNeedSweep(const AABB& a, const AABB& b,
                                       const Vec2& dir) const {
    AABB aabb1 = b;
    aabb1.halfSize += a.halfSize;
    AABB aabb2;
    Vec2 halfDir = dir * 0.5;
    aabb2.center = halfDir + a.center;
    aabb2.halfSize.x = std::abs(halfDir.x);
    aabb2.halfSize.y = std::abs(halfDir.y);

    return IsAABBOverlap(aabb1, aabb2);
}

SweepHitInfo PhysicsScene::sweep(const PhysicActor& actor1,
                                 const PhysicActor& actor2) {
    Vec2 v = actor1.movement_ - actor2.movement_;

    return sweep(actor1.collideShape_, actor2.collideShape_, v);
}

SweepHitInfo PhysicsScene::sweep(const Shape& shape1, const Shape& shape2,
                                 const Vec2& dir) {
    if (FLT_EQ(dir.LengthSqrd(), 0)) {
        return {};
    }

    if (shape1.type != Shape::Type::Circle ||
        shape2.type != Shape::Type::Circle) {
        TL_RETURN_VALUE_IF_FALSE(
            quickCheckNeedSweep(shape1.aabb, shape2.aabb, dir), SweepHitInfo{});
    }

    SweepHitInfo hitInfo;
    if (shape1.type == Shape::Type::Circle) {
        if (shape2.type == Shape::Type::Circle) {
            hitInfo = RaycastByCircle(
                shape1.GetCenter(), dir,
                Circle{shape2.circle.center,
                       shape2.circle.radius + shape1.circle.radius});
        } else if (shape2.type == Shape::Type::AABB) {
            hitInfo = RaycastByRoundAABB(shape1.GetCenter(), dir,
                                         shape1.circle.radius, shape2.aabb);
        }
    } else if (shape1.type == Shape::Type::AABB) {
        if (shape2.type == Shape::Type::Circle) {
            hitInfo = RaycastByRoundAABB(shape2.GetCenter(), -dir,
                                         shape2.circle.radius, shape1.aabb);
            hitInfo.normal *= -1;
        } else if (shape2.type == Shape::Type::AABB) {
            hitInfo = RaycastByAABB(
                shape1.GetCenter(), dir,
                AABB{shape2.aabb.center,
                     shape2.aabb.halfSize + shape1.aabb.halfSize});
        }
    }

    return hitInfo;
}

bool PhysicsScene::checkOverlap(const PhysicActor& trigger,
                                const PhysicActor& solid) const {
    if (trigger.collideShape_.type == Shape::Type::Circle) {
        if (solid.collideShape_.type == Shape::Type::Circle) {
            return IsCircleOverlap(trigger.collideShape_.circle,
                                   solid.collideShape_.circle);
        }
        if (solid.collideShape_.type == Shape::Type::AABB) {
            return IsCircleAABBOverlap(trigger.collideShape_.circle,
                                       solid.collideShape_.aabb);
        }
        return false;
    }

    if (trigger.collideShape_.type == Shape::Type::AABB) {
        if (solid.collideShape_.type == Shape::Type::Circle) {
            return IsCircleAABBOverlap(solid.collideShape_.circle,
                                       trigger.collideShape_.aabb);
        }
        if (solid.collideShape_.type == Shape::Type::AABB) {
            return IsAABBOverlap(solid.collideShape_.aabb,
                                 trigger.collideShape_.aabb);
        }
        return false;
    }
}
} // namespace tl
