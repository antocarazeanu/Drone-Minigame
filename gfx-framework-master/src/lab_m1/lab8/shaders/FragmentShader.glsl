#version 330

// Input
in vec3 world_position;
in vec3 world_normal;

// Uniforms for light properties
uniform vec3 light_direction;
uniform vec3 light_position;
uniform vec3 my_light_position;
uniform vec3 eye_position;

uniform float material_kd;
uniform float material_ks;
uniform int material_shininess;

// Additional uniforms
uniform vec3 object_color; 
uniform int type_of_light; 
uniform float cut_off_angle;

// Output
layout(location = 0) out vec4 out_color;

void main()
{
    vec3 N = normalize(world_normal); 
    vec3 L = normalize(light_position - world_position);
    vec3 myL = normalize(my_light_position - world_position);
    vec3 V = normalize(eye_position - world_position);
    vec3 H = normalize(L + V);
    vec3 myH = normalize(myL + V);

    // Ambient, diffuse, and specular components for primary light
    float ambient_light = 0.25;
    float diffuse_light = material_kd * max(dot(N, L), 0);
    float specular_light = 0;
    
    if (diffuse_light > 0) {
    specular_light = material_ks * pow(max(dot(N, H), 0), material_shininess);
    }

    // Ambient, diffuse, and specular components for secondary light
    float my_diffuse_light = material_kd * max(dot(N, myL), 0); 
    float my_specular_light = 0; 
    
    if (my_diffuse_light > 0) {
    my_specular_light = material_ks * pow(max(dot(N, myH), 0), material_shininess);
    }

    // Spotlight calculations if applicable
    float light = 0;
    if (type_of_light == 1) // Spotlight logic 
    { 
        float cut_off_rad = radians(cut_off_angle); 
        float spot_light = dot(-L, light_direction); 
        float my_spot_light = dot(-myL, light_direction); 
        float spot_light_limit = cos(cut_off_rad); 
         
        // Spotlight effect for the primary light 
        if (spot_light > spot_light_limit) 
        { 
            float linear_att = (spot_light - spot_light_limit) / (1 - spot_light_limit); 
            float light_att_factor = pow(linear_att, 2); 
            light = ambient_light + light_att_factor * (diffuse_light + specular_light); 
        } 
        else 
        { 
            light = ambient_light; // No spotlight effect, only ambient 
        } 
         
        // Spotlight effect for the secondary light 
        if (my_spot_light > spot_light_limit) 
        { 
            float my_linear_att = (my_spot_light - spot_light_limit) / (1 - spot_light_limit); 
            float my_light_att_factor = pow(my_linear_att, 2); 
            light += my_light_att_factor * (my_diffuse_light + my_specular_light); 
        } 
    } else // General lighting (not spotlight)
    {
        float d = length(light_position - world_position); 
        float at = 1.0 / (d * d); 
         
        light = ambient_light + at *(diffuse_light + specular_light)
                + my_diffuse_light + my_specular_light; 
    } 

    // Compute the final color
    vec3 color = object_color * light;

    // Write the output color
    out_color = vec4(color, 1.0);
}