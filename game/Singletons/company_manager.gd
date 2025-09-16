class_name company_manager extends Node

static var singleton_instance: company_manager

var companies: Dictionary[int, Array] #Country id -> List of companies

func _init() -> void:
	assert(singleton_instance == null, "Cannot create multiple instances of singleton!")
	singleton_instance = self

static func get_instance() -> company_manager:
	assert(singleton_instance != null, "Company_manager has not be created, and has been accessed")
	return singleton_instance
