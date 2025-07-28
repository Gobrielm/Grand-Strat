#include "../isolated_broker.hpp"


class SubsistenceFarm: public IsolatedBroker {
    GDCLASS(SubsistenceFarm, IsolatedBroker);

    protected:
    static void _bind_methods();

    public:
    SubsistenceFarm(int p_owner = 0);
};