#pragma once
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
namespace Engine{
    struct Ray{
        glm::vec3 origin;
        glm::vec3 dir;
    };

    inline Ray MakeRayFromMouseNDC(const glm::vec2& ndc, const glm::mat4& proj, const glm::mat4& view) {
        glm::mat4 invVP = glm::inverse(proj * view);

        glm::vec4 nearClip(ndc.x, ndc.y, -1.0f, 1.0f);
        glm::vec4 farClip (ndc.x, ndc.y,  1.0f, 1.0f);

        glm::vec4 nearWorld = invVP * nearClip;
        glm::vec4 farWorld  = invVP * farClip;

        nearWorld /= nearWorld.w;
        farWorld  /= farWorld.w;

        Ray r;

        r.origin = glm::vec3(nearWorld);
        r.dir    = glm::normalize(glm::vec3(farWorld - nearWorld));

        return r;
    }

    inline float DistancePointToRay(const glm::vec3& p, const Ray& r) {
        glm::vec3 v = p - r.origin;
        float t = glm::dot(v, r.dir);
        glm::vec3 closest = r.origin + r.dir * t;
        return glm::length(p - closest);
    }

    inline glm::vec2 MouseToNDC(double mouseX, double mouseY, int winW, int winH) {
        float x =  (2.0f * (float)mouseX) / (float)winW - 1.0f;
        float y =  1.0f - (2.0f * (float)mouseY) / (float)winH; 
        return {x, y};
    }
    
    inline bool WorldToScreen(
        const glm::vec3& world,
        const glm::mat4& view,
        const glm::mat4& proj,
        int w, int h,
        glm::vec2& outScreen
    ) {
        glm::vec4 clip = proj * view * glm::vec4(world, 1.0f);
        if (clip.w <= 0.0001f) return false;

        glm::vec3 ndc = glm::vec3(clip) / clip.w;
        if (ndc.x < -1 || ndc.x > 1 || ndc.y < -1 || ndc.y > 1 || ndc.z < -1 || ndc.z > 1)
            return false;


        outScreen.x = (ndc.x * 0.5f + 0.5f) * (float)w;
        outScreen.y = (1.0f - (ndc.y * 0.5f + 0.5f)) * (float)h; 
        return true;
    }
}