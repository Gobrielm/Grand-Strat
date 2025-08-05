#include "../isolated_broker.hpp"


class SubsistenceFarm: public IsolatedBroker {
    GDCLASS(SubsistenceFarm, IsolatedBroker);

    protected:
    static void _bind_methods();

    public:
    SubsistenceFarm();
    SubsistenceFarm(Vector2i p_location, int p_owner);

    void month_tick() override;
};