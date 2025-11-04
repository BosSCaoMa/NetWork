#ifndef NO_CPOYABLE_H
#define NO_CPOYABLE_H

class nocopyable {
public:
    nocopyable() = default;
    ~nocopyable() = default;

    nocopyable(const nocopyable&) = delete;
    nocopyable& operator=(const nocopyable&) = delete;

    nocopyable(nocopyable&&) = delete;
    nocopyable& operator=(nocopyable&&) = delete;
};

#endif // NO_CPOYABLE_H