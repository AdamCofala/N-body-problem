#version 460 core
in vec3 vPos;
in vec4 v_Color;
out vec4 FragColor;

void main()
{
    // Calculate distance from center of point
    vec2 coord = gl_PointCoord - vec2(0.5);
    float dist = length(coord);
    
    // Apply color normalization as in your original shader
    vec4 normalizedColor = vec4(v_Color.rgb/255.0, v_Color.a/255.0);
    
    // Simple alpha falloff - no discard
    float alpha = 1.0 - smoothstep(0.0, 1.0, dist);
    
    // Output with fading alpha for bloom effect
    FragColor = vec4(normalizedColor.rgb, normalizedColor.a * alpha);
}