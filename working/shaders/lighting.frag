varying vec3 normal;
varying vec3 vertex;
uniform sampler2D texture;

void main(void) {

	// Get distance from light
	float distance = length(gl_LightSource[0].position.xyz - vertex);
	
	// Attenuate
	float attenuation = 1.0f / (0.5 + distance * 0.05 + distance * distance * 0.05);
	attenuation = clamp(attenuation, 0.0, 1.0);
	
	vec3 light_vector = normalize(gl_LightSource[0].position.xyz - vertex); 
	//vec3 eye_vector = normalize(-vertex);
	//vec3 reflect_vector = normalize(-reflect(light_vector, normal)); 

	// Ambient
	vec4 ambient = vec4(gl_LightModel.ambient.x, gl_LightModel.ambient.y, gl_LightModel.ambient.z, 1);
	//vec4 ambient = vec4(0, 0, 0, 1);

	// Diffuse
	vec4 diffuse = gl_LightSource[0].diffuse * max(dot(normal, light_vector), 0.0);

	// Specular
	//vec4 specular_color = vec4(0, 0, 1, 1);
	//vec4 specular = specular_color * pow(max(dot(reflect_vector, eye_vector), 0.0), 3);
					  
	// Texture color
	vec4 texture_color = texture2D(texture, vec2(gl_TexCoord[0]));
	
	// Final color
	vec4 frag_color = texture_color * (ambient + attenuation * diffuse); // + specular);
	//vec4 frag_color = vec4(0, 0, 0, 1);
	
	// Fog
	if(gl_Fog.density > 0) {
		const float LOG2 = 1.442695;
		float frag_z = gl_FragCoord.z / gl_FragCoord.w;
		float fog_factor = clamp(exp2(-gl_Fog.density * gl_Fog.density * frag_z * frag_z * LOG2), 0.0, 1.0);
	
		gl_FragColor = mix(gl_Fog.color, frag_color, fog_factor);
	}
	else {
		gl_FragColor = frag_color;
	}
}
