#include "register_types.h"
#include <gdextension_interface.h>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

#include "static_registry.hpp"
#include "../classes/province.hpp"
#include "../classes/trade_order.hpp"
#include "../classes/terminal.hpp"
#include "../classes/firm.hpp"
#include "../classes/hold.hpp"
#include "../classes/fixed_hold.hpp"
#include "../classes/broker.hpp"
#include "../classes/station.hpp"
#include "../classes/town.hpp"
#include "../classes/factory_template.hpp"
#include "../classes/construction_site.hpp"
#include "../classes/factory.hpp"
#include "../classes/ai_factory.hpp"
#include "../classes/isolated_broker.hpp"
#include "../classes/Specific_Buildings/subsistence_farm.hpp"
#include "../classes/road_depot.hpp"
#include "../classes/ai_base.hpp"
#include "../classes/company_ai.hpp"
#include "../classes/prospector_ai.hpp"
#include "../classes/initial_builder.hpp"
#include "../singletons/money_controller.hpp"
#include "../singletons/road_map.hpp"
#include "../singletons/terminal_map.hpp"
#include "../singletons/data_collector.hpp"
#include "../singletons/province_manager.hpp"
#include "../singletons/factory_creator.hpp"
#include "../singletons/user_singletons/country_manager.hpp"
#include "../singletons/user_singletons/country.hpp"



using namespace godot;

void initialize_gdextension_types(ModuleInitializationLevel p_level)
{

	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	//--verbose in godot for more details
	GDREGISTER_CLASS(MoneyController);
	GDREGISTER_CLASS(Province);
	GDREGISTER_CLASS(TradeOrder);
	GDREGISTER_CLASS(Terminal);
	GDREGISTER_CLASS(Firm);
	GDREGISTER_CLASS(Hold);
	GDREGISTER_CLASS(FixedHold);
	GDREGISTER_CLASS(Broker);
	GDREGISTER_CLASS(StationWOMethods);
	GDREGISTER_CLASS(Town);
	GDREGISTER_CLASS(FactoryTemplate);
	GDREGISTER_CLASS(ConstructionSite);
	GDREGISTER_CLASS(Factory);
	GDREGISTER_CLASS(AiFactory);
	GDREGISTER_CLASS(IsolatedBroker);
	GDREGISTER_CLASS(SubsistenceFarm);
	GDREGISTER_CLASS(RoadDepot);
	GDREGISTER_CLASS(AiBase);
	GDREGISTER_CLASS(CompanyAi);
	GDREGISTER_CLASS(ProspectorAi);
	GDREGISTER_CLASS(InitialBuilder);
	GDREGISTER_CLASS(CargoInfo);
	GDREGISTER_CLASS(RoadMap);
	GDREGISTER_CLASS(TerminalMap);
	GDREGISTER_CLASS(DataCollector);
	GDREGISTER_CLASS(ProvinceManager);
	GDREGISTER_CLASS(FactoryCreator);
	GDREGISTER_CLASS(Country);
	GDREGISTER_CLASS(CountryManager);
	StaticRegistry::initialize();
	
}


void uninitialize_gdextension_types(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	StaticRegistry::uninitialize();
}

extern "C"
{
	// Initialization
	GDExtensionBool GDE_EXPORT grand_strat_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization)
	{
		GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);
		init_obj.register_initializer(initialize_gdextension_types);
		init_obj.register_terminator(uninitialize_gdextension_types);
		init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

		return init_obj.init();
	}
}