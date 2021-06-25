typedef struct {
    float x, y;
    bool solid;
} Particle;

// Hash function www.cs.ubc.ca/~rbridson/docs/schechter-sca08-turbulence.pdf
uint hash(uint state, uint random) {
    state ^= random;
    state *= random;
    state ^= 2747636419u;
    state *= 2654435769u;
    state ^= state >> 16;
    state *= 2654435769u;
    state ^= state >> 16;
    state *= 2654435769u;
    return state;
}

__kernel void particle_move(__global Particle* particles, __constant int* random) {
    // Get the index of the current element to be processed
    int i = get_global_id(0);
    // P E M-D A-S
    float val = (hash(particles[i].x, (uint)random[0]) / 4294967295.0 * 2.0) - 1.0;
    particles[i].x += val;

    val = (hash(particles[i].y, (uint)random[0]) / 4294967295.0 * 2.0) - 1.0;
    particles[i].y += val;

    if (particles[i].x < 0.f) {
        particles[i].x = 1920.0;
    }
    else if (particles[i].x > 1920.0) {
        particles[i].x = 0.f;
    }

    if (particles[i].y < 0.f) {
        particles[i].y = 1080.0;
    }
    else if (particles[i].y > 1080.0) {
        particles[i].y = 0.f;
    }
}