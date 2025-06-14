#include "register_types.h"
#include <gdextension_interface.h>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

#include "static_registry.hpp"
#include "../classes/base_pop.hpp"
#include "../classes/rural_pop.hpp"
#include "../classes/town_pop.hpp"
#include "../classes/province.hpp"
#include "../classes/trade_order.hpp"
#include "../classes/local_price_controller.hpp"
#include "../classes/factory_local_price_controller.hpp"
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
#include "../classes/private_ai_factory.hpp"
#include "../classes/road_depot_wo_methods.hpp"
#include "../classes/ai_base.hpp"
#include "../classes/company_ai.hpp"
#include "../singletons/money_controller.hpp"
#include "../singletons/road_map.hpp"
#include "../singletons/terminal_map.hpp"
#include "../singletons/data_collector.hpp"
#include "../singletons/province_manager.hpp"




using namespace godot;

void initialize_gdextension_types(ModuleInitializationLevel p_level)
{

	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	//--verbose in godot for more details
	GDREGISTER_CLASS(LocalPriceController);
	GDREGISTER_CLASS(FactoryLocalPriceController);
	GDREGISTER_CLASS(MoneyController);
	GDREGISTER_CLASS(BasePop);
	GDREGISTER_CLASS(RuralPop);
	GDREGISTER_CLASS(TownPop);
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
	GDREGISTER_CLASS(PrivateAiFactory);
	GDREGISTER_CLASS(RoadDepotWOMethods);
	GDREGISTER_CLASS(AiBase);
	GDREGISTER_CLASS(CompanyAi);
	GDREGISTER_CLASS(CargoInfo);
	GDREGISTER_CLASS(RoadMap);
	GDREGISTER_CLASS(TerminalMap);
	GDREGISTER_CLASS(DataCollector);
	GDREGISTER_CLASS(ProvinceManager);
	StaticRegistry::initialize();
	
}


void uninitialize_gdextension_types(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
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