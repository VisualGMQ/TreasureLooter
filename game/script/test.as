class MyVec2 {
    float x;
    float y;

    MyVec2(float x, float y) {
        this.x = x;
        this.y = y;
    }
}

// MyVec2 opSub(const MyVec2& v1, const MyVec2& v2) {
//     return MyVec2(v1.x - v2.x, v1.y - v2.y);
// }

class MyClass {
    void OnInit() {
        TL::Log("Initialize");
        TL::Vec2 v = TL::Vec2(1, 2);
        TL::Log(v.Length());
        TL::Log(v.Dot(TL::Vec2(2, 3)));
        TL::Log(v.x);
        TL::Log(v.y);

        MyVec2 mv1(1, 2);
        MyVec2 mv2(1, 2);

        // TL::Vec2 v2 = TL::Vec2(3, 54);
        // v == TL::Vec2(1, 2);
        TL::Vec2 v3 = 3 * v;
        TL::Log(v3.x);
        TL::Log(v3.y);

        TL::Color color;
        TL::Log(color.r);
        TL::Log(TL::Color::Red.r);
    }

    void OnUpdate(float delta_time) {
    }

    void OnQuit() {
        TL::Log("Quit");
    }
}
