#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <mutex>
#include <shared_mutex>

enum class CallState { INVALID = -1,
                       IDLE,
                       INCOMING,
                       OUTGOING,
                       ACTIVE,
                       HELDACTIVE,
                       HELDNOACTIVE,
                       INSECONDCALL,
                       OUTSECONDCALL };

static const char *sCallStates[] = {
    "INVALID",
    "IDLE",
    "INCOMING",
    "OUTGOING",
    "ACTIVE",
    "HELDACTIVE",
    "HELDNOACTIVE",
    "INSECONDCALL",
    "OUTSECONDCALL",
};

enum class CallLevel {
    FIRST_CALL = 0,
    SECOND_CALL
};

enum class CallType {
    UNKNOWN = -1,
    IN_CALL = 0,
    OUT_CALL,
};

class Call
{
public:
    int id;
    std::string name;
    std::string number;
    std::string portrait;
    int seconds;
    bool active;
    std::string time;
    CallState state;
    CallLevel level;
    CallType type;
    int32_t index;
    Call() {
        this->id = -1;
        this->name = "";
        this->number = "";
        this->portrait = "";
        this->seconds = 0;
        this->state = CallState::INVALID;
        this->active = false;
    }
    Call(int id, int32_t index, CallType type, std::string name, std::string number, std::string portrait, int seconds, CallState state) {
        this->id = id;
        this->index = index;
        this->type = type;
        this->name = name;
        this->number = number;
        this->portrait = portrait;
        this->seconds = seconds;
        this->state = state;
        this->active = false;
    }

    bool isActive() { return active; }

    bool alive() { return (this->state != CallState::IDLE && this->state != CallState::INVALID); }

    bool incomming() { return (this->state == CallState::INCOMING || this->state == CallState::INSECONDCALL); }

    void terminate() {
        this->state = CallState::IDLE;
        this->active = false;
    }
    void answer() {
        if (this->type == CallType::OUT_CALL) return;

        if (this->state == CallState::INCOMING ||
            this->state == CallState::INSECONDCALL) {
            this->state = CallState::ACTIVE;
            this->active = true;
        }
    }

    void dialAnswer() {
        if (this->type == CallType::IN_CALL) return;

        if (this->state == CallState::OUTGOING ||
            this->state == CallState::OUTSECONDCALL) {
            this->state = CallState::ACTIVE;
            this->active = true;
        }
    }

    void switchCall() {
        if (this->state == CallState::ACTIVE) {
            if (level == CallLevel::FIRST_CALL) {
                this->state = CallState::HELDACTIVE;
            } else {
                this->state = CallState::HELDNOACTIVE;
            }
            this->active = false;
        } else if (this->state == CallState::INCOMING ||
                   this->state == CallState::INSECONDCALL ||
                   this->state == CallState::OUTGOING ||
                   this->state == CallState::OUTSECONDCALL) {
            this->active = false;
        } else {
            this->state = CallState::ACTIVE;
            this->active = true;
        }
    }
    void reject() {
        if (this->type == CallType::OUT_CALL) return;

        if (this->state == CallState::INCOMING ||
            this->state == CallState::INSECONDCALL) {
            this->state = CallState::IDLE;
        }
    }

    void printInfo() {
        printf("[%p]: id %d, type %d, name %s, number %s, portrait %s, state %s, second %d\n",
               this, this->id, (int)this->type, this->name.c_str(), this->number.c_str(),
               this->portrait.c_str(), sCallStates[(int)this->state + 1], this->seconds);
    }
};

static Call sCalls[] = {
    Call(1, 10, CallType::UNKNOWN, "John Doe", "555-1234", "portrait1.jpg", 0, CallState::INVALID),
    Call(2, 10, CallType::UNKNOWN, "Waston Smith", "525-5678", "portrait2.jpg", 0, CallState::INVALID),
    Call(2, 10, CallType::UNKNOWN, "Jorden Smith", "1235-5678", "portrait2.jpg", 0, CallState::INVALID),
    Call(2, 10, CallType::UNKNOWN, "Diana Smith", "3455-5678", "portrait2.jpg", 0, CallState::INVALID),
    Call(2, 10, CallType::UNKNOWN, "Hana Smith", "453-5678", "portrait2.jpg", 0, CallState::INVALID),
    Call(2, 10, CallType::UNKNOWN, "Broline Smith", "5675-5678", "portrait2.jpg", 0, CallState::INVALID),
    Call(2, 10, CallType::UNKNOWN, "Aley Smith", "555-5678", "portrait2.jpg", 0, CallState::INVALID),
    Call(2, 10, CallType::UNKNOWN, "Kevin Smith", "555-5678", "portrait2.jpg", 0, CallState::INVALID),
    Call(2, 10, CallType::UNKNOWN, "Adam Smith", "555-5678", "portrait2.jpg", 0, CallState::INVALID),
    Call(2, 10, CallType::UNKNOWN, "Will Smith", "555-5678", "portrait2.jpg", 0, CallState::INVALID),
    Call(3, 10, CallType::UNKNOWN, "Bob Johnson", "555-9876", "portrait3.jpg", 0, CallState::INVALID)};

class CallManager
{
private:
    int counter;
    std::vector<Call *> calls;
    Call *originalCall;
    Call *secondCall;
    CallState currentState;
    std::shared_mutex mtx;

public:
    CallManager() :
        originalCall(NULL), secondCall(NULL) {
        currentState = CallState::INVALID;
        counter = 0;
    }
    ~CallManager() {
        for (auto call : calls) {
            delete call;
        }
    }

    CallState state() {
        std::shared_lock locker(mtx);
        return currentState;
    }

    int bussy() {
        std::shared_lock locker(mtx);
        return (originalCall && secondCall);
    }

    Call *getActiveCall() {
        std::shared_lock locker(mtx);
        if (originalCall)
            if (originalCall->isActive()) return originalCall;
        if (secondCall)
            if (secondCall->isActive()) return secondCall;
        return NULL;
    }

    Call *getWaitingCall() {
        std::shared_lock locker(mtx);
        if (originalCall)
            if (originalCall->alive()) return originalCall;
        if (secondCall)
            if (secondCall->alive()) return secondCall;
        return NULL;
    }

    Call *getIncomCall() {
        std::shared_lock locker(mtx);
        if (originalCall)
            if (originalCall->incomming()) return originalCall;
        if (secondCall)
            if (secondCall->incomming()) return secondCall;
        return NULL;
    }

    void generateInCall() {
        std::srand(std::time(nullptr));
        int index = rand() % sizeof(sCalls) / sizeof(sCalls[0]);
        Call *dumy = &sCalls[index];

        if (bussy()) return;
        std::unique_lock locker(mtx);
        Call *call = new Call(++counter, counter, CallType::IN_CALL, dumy->name, dumy->number, dumy->portrait, 0, CallState::INVALID);
        if (originalCall) {
            secondCall = call;
            call->level = CallLevel::SECOND_CALL;
            call->state = CallState::INSECONDCALL;
        } else {
            originalCall = call;
            call->level = CallLevel::FIRST_CALL;
            call->state = CallState::INCOMING;
        }
        currentState = call->state;
        calls.push_back(call);
    }

    void generateOutCall() {
        std::srand(std::time(nullptr));
        int index = rand() % sizeof(sCalls) / sizeof(sCalls[0]);
        Call *dumy = &sCalls[index];

        if (bussy()) return;
        std::unique_lock locker(mtx);
        Call *call = new Call(++counter, counter, CallType::OUT_CALL, dumy->name, dumy->number, dumy->portrait, 0, CallState::INVALID);
        if (originalCall) {
            secondCall = call;
            call->level = CallLevel::FIRST_CALL;
            call->state = CallState::OUTSECONDCALL;
        } else {
            originalCall = call;
            call->level = CallLevel::FIRST_CALL;
            call->state = CallState::OUTGOING;
        }
        currentState = call->state;
        calls.push_back(call);
    }

    void dialout(int id, std::string &number, std::string &name) {
        std::unique_lock locker(mtx);
        if (originalCall && secondCall) return;
        Call *call = new Call(++counter, counter, CallType::OUT_CALL, name, number, "", 0, CallState::INVALID);
        if (originalCall) {
            secondCall = call;
            call->level = CallLevel::FIRST_CALL;
            call->state = CallState::OUTSECONDCALL;
        } else {
            originalCall = call;
            call->level = CallLevel::FIRST_CALL;
            call->state = CallState::OUTGOING;
        }
        currentState = call->state;
        calls.push_back(call);
    }

    void terminate() {
        auto call = getActiveCall();
        if (!call) {
            auto incom = getIncomCall();
            if (incom) {
                std::unique_lock locker(mtx);
                incom->reject();
                originalCall = NULL;
                secondCall = NULL;
            }
        } else {
            {
                std::unique_lock locker(mtx);
                call->terminate();
                currentState = call->state;
            }

            auto other = getWaitingCall();
            auto incom = getIncomCall();
            std::unique_lock locker(mtx);
            if (incom) {
                incom->switchCall();
                originalCall = incom;
                originalCall->level = CallLevel::FIRST_CALL;
                incom->state = CallState::INCOMING;
                currentState = incom->state;
                secondCall = NULL;
            } else if (other) {
                other->switchCall();
                originalCall = other;
                originalCall->level = CallLevel::FIRST_CALL;
                currentState = other->state;
                secondCall = NULL;
            } else {
                originalCall = NULL;
                secondCall = NULL;
            }
        }
    }
    void answer() {
        auto active = getActiveCall();
        auto call = getIncomCall();

        if (active) {
            switchCall();
            call = getIncomCall();
        }
        if (call) {
            std::unique_lock locker(mtx);
            call->answer();
            currentState = call->state;
        }
    }
    void switchCall() {
        if (bussy()) {
            if (originalCall->isActive()) {
                std::unique_lock locker(mtx);
                originalCall->switchCall();
                secondCall->switchCall();
                currentState = originalCall->state;
            } else {
                std::unique_lock locker(mtx);
                originalCall->switchCall();
                secondCall->switchCall();
                currentState = secondCall->state;
            }
        }
    }
    void reject() {
        auto call = getIncomCall();
        if (call) {
            {
                std::unique_lock locker(mtx);
                call->reject();
                currentState = call->state;
            }
            auto active = getActiveCall();
            auto other = getIncomCall();

            std::unique_lock locker(mtx);
            if (!active && other) {
                currentState = other->state;
                originalCall = other;
                secondCall = NULL;
            } else if (active && !other) {
                currentState = active->state;
                originalCall = active;
                secondCall = NULL;
            } else {
                originalCall = NULL;
                secondCall = NULL;
            }
        }
    }

    void pickup(int call) {
        std::shared_lock locker(mtx);
        if (call == 1)
            if (originalCall) {
                if (originalCall->type == CallType::OUT_CALL) {
                    if (!originalCall->isActive()) {
                        originalCall->dialAnswer();
                        currentState = originalCall->state;
                        return;
                    }
                }
            }
        if (call == 2)
            if (secondCall) {
                if (secondCall->type == CallType::OUT_CALL) {
                    if (!secondCall->isActive()) {
                        secondCall->dialAnswer();
                        currentState = secondCall->state;
                    }
                }
            }
    }

    void printInfo() {
        std::shared_lock locker(mtx);
        if (originalCall) originalCall->printInfo();
        if (secondCall) secondCall->printInfo();
    }

    void printHis() {
        std::shared_lock locker(mtx);
        for (auto call : calls) {
            call->printInfo();
        }
    }
};