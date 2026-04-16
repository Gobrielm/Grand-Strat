#include <unordered_map>
#include <classes/base_pop.hpp>

class Recipe;

class EmployerComponent {

    std::unordered_map<PopTypes, std::vector<int>> employees;
    std::unordered_map<PopTypes, int> pops_needed;

    std::vector<int> pops_to_fire;

    bool does_need_pop_type(PopTypes pop_type) const;
    void remove_pop(int pop_id, PopTypes pop_type);

    public:

    Recipe recipe;

    EmployerComponent();
    EmployerComponent(Recipe recipe, std::unordered_map<PopTypes, int> p_pops_needed);
    EmployerComponent(const EmployerComponent& other);

    EmployerComponent operator=(const EmployerComponent& other) const;
    EmployerComponent operator=(EmployerComponent other) const;
    
    void upgrade();
    void degrade();


    bool is_pop_type_needed(PopTypes pop_type) const;
    void add_pop(BasePop* pop);
    void add_pop(PopTypes pop_type, int pop_id);
    
    int get_employement() const;
    int get_pops_needed_num() const;
    float get_employment_rate() const;
    void queue_employees_to_be_fired();

    double get_level() const;
    int get_level_without_employment() const;

    std::unordered_map<PopTypes, int> get_pops_needed() const;
    std::unordered_map<int, PopTypes> get_employee_ids() const;
    std::vector<int> get_pops_to_fire() const;

    void set_pops_needed(const std::unordered_map<PopTypes, int> new_pops_needed);
};