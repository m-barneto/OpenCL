#pragma once
#include <thread>

struct Particle {
    cl_float x, y, hue;
    cl_bool solid;
};


class ParticleSystem {
private:
    int particleCount;
    std::vector<Particle> active, solid;
    Vector2u size;
    VertexArray vertexArray;
    std::vector<Vertex> offsets;
    int vertexCount;
    float circleRadius;

    OpenCLHelper ocl;
    int err;

    Kernel collisionKernel;
    cl_mem activeBuffer;
    cl_mem solidBuffer;
    cl_mem solidLengthBuffer;
    std::vector<cl_int> l;

    std::default_random_engine eng;
    std::uniform_real_distribution<float> distx;
    std::uniform_real_distribution<float> disty;
public:
    ParticleSystem(int particleCount, Vector2u size) {
        this->err = CL_SUCCESS;
        this->particleCount = particleCount;
        this->size = size;
        this->vertexArray = VertexArray(PrimitiveType::Triangles);

        ocl.createKernel("collide.cl", "main", collisionKernel);
        

        #pragma region Cache Particle Shape

        this->circleRadius = 3.f;
        this->vertexCount = 5;
        
        float x = 0.f;
        float y = 0.f;

        for (int i = 0; i < vertexCount; ++i) {
            // Center point
            Vertex vert({ x, y }, Color::Cyan);
            offsets.push_back(vert);

            // Previous point
            float angle = (float)(i - 1) / vertexCount * 360.f * PI / 180.f;
            vert.position.x += cosf(angle) * circleRadius;
            vert.position.y += sinf(angle) * circleRadius;
            offsets.push_back(vert);

            // Current Point
            vert = Vertex({ x, y }, Color::Cyan);
            angle = (float)i / vertexCount * 360.f * PI / 180.f;
            vert.position.x += cosf(angle) * circleRadius;
            vert.position.y += sinf(angle) * circleRadius;
            offsets.push_back(vert);
        }
        #pragma endregion
        #pragma region Setup Particles
        std::random_device rd;
        eng = std::default_random_engine(rd());
        distx = std::uniform_real_distribution<float>(0.f, size.x);
        disty = std::uniform_real_distribution<float>(0.f, size.y);

        Particle p;
        for (int i = 0; i < particleCount; ++i) {
            p.x = distx(eng);
            p.y = disty(eng);
            p.solid = false;

            active.push_back(p);
        }

        p.x = size.x / 2.f;
        p.y = size.y / 2.f;
        p.solid = true;

        solid.push_back(p);
        #pragma endregion

        solidBuffer = ocl.createBuffer<Particle>(solid, CL_MEM_WRITE_ONLY);
        l = { (cl_int)solid.size() };
        solidLengthBuffer = ocl.createBuffer<cl_int>(l, CL_MEM_WRITE_ONLY);
        activeBuffer = ocl.createBuffer<Particle>(active, CL_MEM_READ_WRITE);
    }

    void update(const float& deltaTime) {
        // Loop through all actives and update them
        for (int i = 0; i < active.size(); ++i) {
            active[i].x += ((float)rand() / (float)RAND_MAX * 2.f - 1.f);
            active[i].y += ((float)rand() / (float)RAND_MAX * 2.f - 1.f);

            if (active[i].x < 0.f) {
                active[i].x = size.x;
            }
            else if (active[i].x > size.x) {
                active[i].x = 0.f;
            }

            if (active[i].y < 0.f) {
                active[i].y = size.y;
            }
            else if (active[i].y > size.y) {
                active[i].y = 0.f;
            }
        }


        err = ocl.writeToBuffer(collisionKernel, solid, solidBuffer);
        err = ocl.writeArg<cl_mem>(collisionKernel, 0, solidBuffer);

        l[0] = solid.size();
        err = ocl.writeToBuffer(collisionKernel, l, solidLengthBuffer);
        err = ocl.writeArg<cl_mem>(collisionKernel, 1, solidLengthBuffer);

        err = ocl.writeToBuffer(collisionKernel, active, activeBuffer);
        err = ocl.writeArg<cl_mem>(collisionKernel, 2, activeBuffer);

        err = ocl.runKernel(collisionKernel, active.size(), 1000);

        err = ocl.readArg<Particle>(2, activeBuffer, &active[0], active.size());

        // Loop through actives and if they're solid, move them to the solid vector
        for (int i = active.size() - 1; i >= 0; --i) {
            if (active[i].solid) {
                // Remove from active, move to solid
                solid.push_back(active[i]);
                active.erase(active.begin() + i);
            }
        }
        printf("Active: %i\nSolid: %i\n", active.size(), solid.size());
    }

    void render(RenderWindow& window, const float& dT) {
        window.setTitle(std::to_string(solid.size()));

        vertexArray.resize((active.size() + solid.size()) / 2 * vertexCount * 3 * 3);
        Color col = Color::Blue;
        for (int i = 0; i < active.size(); ++i) {
            Vector2f v{ active[i].x, active[i].y };
            for (int j = 0; j < vertexCount * 3; j += 3) {
                vertexArray.append(Vertex{ offsets[j].position + v, col });
                vertexArray.append(Vertex{ offsets[j + 1].position + v, col });
                vertexArray.append(Vertex{ offsets[j + 2].position + v, col });
            }
        }
        col = Color::Cyan;
        for (int i = 0; i < solid.size(); ++i) {
            Vector2f v{ solid[i].x, solid[i].y };
            for (int j = 0; j < vertexCount * 3; j += 3) {
                vertexArray.append(Vertex{ offsets[j].position + v, col });
                vertexArray.append(Vertex{ offsets[j + 1].position + v, col });
                vertexArray.append(Vertex{ offsets[j + 2].position + v, col });
            }
        }

        window.draw(vertexArray);
    }
};