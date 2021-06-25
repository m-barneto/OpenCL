
typedef struct {
    float x, y, hue;
    bool solid;

} Particle;

// TODO SEPARATE INTO 2 VECTORS, ONE SOLID, ONE ACTIVE
__kernel void main(constant Particle* solid, constant uint* length, global Particle* active) {
    int i = get_global_id(0);

    active[i].x = length * 100.0;

    // Loop through all solids
    for (int j = 0; j < length[0]; ++j) {
        float dist = (pow(active[i].x - solid[j].x, 2) + pow(active[i].y - solid[j].y, 2));
        if (dist < 30.0) {
            //active[i].solid = true;
            return;
        }
    }
    /*
    for (int i = o + 1; i < size; ++i) {
        //if (i == o) continue; //!particles[i].solid ||

        float dist = (((particles[o].x - particles[i].x) * (particles[o].x - particles[i].x)) + ((particles[o].y - particles[i].y) * (particles[o].y - particles[i].y)));
        if (dist < 16.0) {
            if (particles[o].solid) {
                particles[i].solid = true;
            }
            else if (particles[i].solid) {
                particles[o].solid = true;
            }
            break;
        }
    }

    for (int i = 0; i < size; ++i) {
        if (i == o || !particles[i].solid) continue;
        float dist = (pow(particles[o].x - particles[i].x, 2) + pow(particles[o].y - particles[i].y, 2));
        if (dist < 16.0) {
            particles[o].solid = true;
        }
    }
    */

}