namespace TL {

shared class Behavior: IBehavior {
    Behavior(Entity entity) {
        m_entity = entity;
    }

    Entity GetEntity() const override {
        return m_entity;
    }

    void OnInit() override {}
    void OnUpdate(TimeType) override {}
    void OnQuit() override {}

    // get other component

    AnimationPlayer@ GetAnimationPlayerComponentFrom(Entity entity) {
		return GetGameContext().m_animation_player_manager.Get(entity);
    }

    Transform@ GetTransformComponentFrom(Entity entity) {
		return GetGameContext().m_transform_manager.Get(entity);
    }

    Sprite@ GetSpriteComponentFrom(Entity entity) {
		return GetGameContext().m_sprite_manager.Get(entity);
    }

    TilemapComponent@ GetTilemapComponentFrom(Entity entity) {
		return GetGameContext().m_tilemap_component_manager.Get(entity);
    }

    UIWidget@ GetUIComponentFrom(Entity entity) {
		return GetGameContext().m_ui_manager.Get(entity);
    }

    CharacterController@ GetCCTComponentFrom(Entity entity) {
		return GetGameContext().m_cct_manager.Get(entity);
    }

    GameplayConfig@ GetGameplayConfigComponentFrom(Entity entity) 
    {
		return GetGameContext().m_gameplay_config_manager.Get(entity);
    }

    Relationship@ GetRelationshipComponentFrom(Entity entity) {
		return GetGameContext().m_relationship_manager.Get(entity);
    }

    // get myself components

    AnimationPlayer@ GetAnimationPlayerComponent() {
		return GetAnimationPlayerComponentFrom(m_entity);
    }

    Transform@ GetTransformComponent() {
		return GetTransformComponentFrom(m_entity);
    }

    Sprite@ GetSpriteComponent() {
		return GetSpriteComponentFrom(m_entity);
    }

    TilemapComponent@ GetTilemapComponent() {
		return GetTilemapComponentFrom(m_entity);
    }

    UIWidget@ GetUIComponent() {
		return GetUIComponentFrom(m_entity);
    }

    CharacterController@ GetCCTComponent() {
		return GetCCTComponentFrom(m_entity);
    }

    GameplayConfig@ GetGameplayConfigComponent() 
    {
		return GetGameplayConfigComponentFrom(m_entity);
    }

    Relationship@ GetRelationshipComponent() {
		return GetRelationshipComponentFrom(m_entity);
    }
	
    Behavior@ GetBehavior(Entity entity) {
        IBehavior@ a = GetGameContext().m_script_manager.Get(entity);
        return cast<Behavior>(a);
    }

    private Entity m_entity;
}

}
