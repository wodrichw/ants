template <typename Event>
struct Subscriber
{
    virtual void update(Event event) = 0;
};
