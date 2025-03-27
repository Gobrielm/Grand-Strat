shader_type canvas_item;

uniform int onoff;

void vertex() {
	// Called for every vertex the material is visible on.
}

void fragment() {
	// Called for every pixel the material is visible on.
	if (onoff == 1) {
		COLOR.rgb = vec3(0.2126 * COLOR.r + 0.7152 * COLOR.g + 0.0722 * COLOR.b);
	}

}

//void light() {
	// Called for every pixel for every light affecting the CanvasItem.
	// Uncomment to replace the default light processing function with this one.
//}
