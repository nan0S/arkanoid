#version 330 core

in vec2 position;
uniform float time;
out vec3 color;

const int a = 50;

float constrain(float value, float value_min, float value_max, float target_min, float target_max)
{
    float value_d = (value_max - value_min);
    float target_d = (target_max - target_min);
    return (value - value_min) / value_d * target_d + target_min;
}

void main()
{
    int x = int(floor(gl_FragCoord.x)) % a - a / 2;
    int y = int(floor(gl_FragCoord.y)) % a - a / 2;

    if (x + y > 0)
    {
        if (x - y > 0)
            color = vec3(2.5f / 255, 15.0f / 255, 31.0f / 255);
        else
            color = vec3(18.5f / 255, 15.0f / 255, 31.0f / 255);
    }
    else
    {
        if (x - y > 0)
            color = vec3(34.5f / 255, 15.0f / 255, 31.0f / 255);
        else
            color = vec3(50.5f / 255, 15.0f / 255, 31.0f / 255);
    }

    float mult = abs(cos(time)) * (1.0f - abs(position.x));
    mult = constrain(mult, 0.0f, 1.0f, 0.6f, 1.0f);
    color *= mult * 5.0f;
}
