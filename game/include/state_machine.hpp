#pragma once

/**
 * pure functional class, can't contain any data
 */
template <typename T>
class State {
public:
    friend class StateSingletonManager;
    
    using type = T;

    State(const State& other) = delete;
    State(State&& other) = delete;
    State& operator=(const State& other) = delete;
    State& operator=(State&& other) = delete;

    virtual ~State() = default;

    virtual void OnEnter(T* state) {}

    virtual void OnQuit(T* state) {}

    virtual void OnUpdate(T* state) {}

private:
    State() = default;
};

/**
 * use to create and get state singleton
 */
class StateSingletonManager {
public:
    template <typename T>
    static T& GetState() {
        static T state;
        return state;
    }
};

template <typename T>
class StateMachine {
public:
    using payload_type = T;
    using state_type = State<T>;

    void ChangeState(state_type* state) { m_next_state = state; }

    void ExitAllState() { ChangeState(nullptr); }

    void OnUpdate() {
        if (m_next_state) {
            if (m_current_state) {
                m_current_state->OnQuit(m_payload);
            }
            m_next_state->OnEnter(m_payload);
            m_current_state = m_next_state;
            m_next_state = nullptr;
        }

        if (m_current_state) {
            m_current_state->OnUpdate(m_payload);
        }
    }

private:
    state_type* m_current_state{};
    state_type* m_next_state{};

    payload_type* m_payload{};
};