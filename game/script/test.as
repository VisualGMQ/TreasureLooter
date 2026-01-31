class MyClass : TL::Behavior {
    MyClass(TL::Entity entity) {
        super(entity);
    }

    void OnInit() {
        TL::Log("Initialize");
    }

    void OnUpdate(float delta_time) {
    }

    void OnQuit() {
        TL::Log("Quit");
    }
}
