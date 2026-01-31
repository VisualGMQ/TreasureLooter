namespace TL {

shared class Behavior {
    Behavior(Entity entity) {
        m_entity = entity;
    }

    Entity GetEntity() const {
        return m_entity;
    }

    void OnInit() {}
    void OnUpdate(float) {}
    void OnQuit() {}

    private Entity m_entity;
}

}