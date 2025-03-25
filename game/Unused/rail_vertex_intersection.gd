#class_name rail_vertex_intersection extends rail_vertex
#
#func _init(endpoint: rail_vertex):
	#location = endpoint.location
	#connections = endpoint.connections
	#for vertex: rail_vertex in connections:
		#vertex.add_connection(self, vertex.get_direction(endpoint), vertex.get_length(endpoint))
		#vertex.remove_connection(endpoint)
	#endpoint.queue_free()
#
#func move_vertex(_new_location: Vector2i):
	#print("Error, cannot move intersection")
