#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <algorithm> // ضرورية لدالة std::remove_if
#include <cstdint>   // [جديد] ضرورية لاستخدام std::uint8_t بدلاً من sf::Uint8

// -----------------------------------------------------------
// تعريف أنواع الانفجارات
// -----------------------------------------------------------
enum class ExplosionType
{
    SmallHit,      // انفجار صغير (إصابة رصاصة)
    BigExplosion   // انفجار كبير (تحطم طائرة)
};

// -----------------------------------------------------------
// هيكل الجزيء الواحد (Particle)
// -----------------------------------------------------------
struct Particle
{
    sf::CircleShape shape;   // شكل الجزيء
    sf::Vector2f velocity;   // سرعة الحركة (x, y)
    float lifetime;          // الزمن المتبقي للموت
    float totalLifetime;     // الزمن الكلي (لحساب نسبة الشفافية)
};

// -----------------------------------------------------------
// هيكل الانفجار (Explosion)
// -----------------------------------------------------------
struct Explosion
{
    std::vector<Particle> particles;
    bool alive = true;
};

// -----------------------------------------------------------
// نظام الانفجارات (ExplosionSystem)
// -----------------------------------------------------------
class ExplosionSystem
{
public:
    // دالة لإضافة انفجار جديد
    void addExplosion(const sf::Vector2f& position, ExplosionType type)
    {
        Explosion explosion;

        int particleCount = 0;
        float minSpeed = 0.f; float maxSpeed = 0.f;
        float minRadius = 0.f; float maxRadius = 0.f;
        float minLifetime = 0.f; float maxLifetime = 0.f;
        bool addSmoke = false;

        if (type == ExplosionType::SmallHit)
        {
            particleCount = 20;
            minSpeed = 100.f; maxSpeed = 300.f;
            minRadius = 1.5f; maxRadius = 3.f;
            minLifetime = 0.1f; maxLifetime = 0.4f;
            addSmoke = false;
        }
        else // BigExplosion
        {
            particleCount = 80;
            minSpeed = 50.f; maxSpeed = 200.f;
            minRadius = 3.f; maxRadius = 7.f;
            minLifetime = 0.5f; maxLifetime = 1.2f;
            addSmoke = true;
        }

        // 1. إنشاء الجزيئات النارية
        for (int i = 0; i < particleCount; ++i)
        {
            Particle p;
            float radius = randomRange(minRadius, maxRadius);
            p.shape.setRadius(radius);
            
            // [تصحيح SFML 3.0] استخدام الأقواس {} لتمرير Vector2f
            p.shape.setOrigin({radius, radius}); 
            p.shape.setPosition(position);

            float angle = randomRange(0.f, 2.f * 3.14159265f);
            float speed = randomRange(minSpeed, maxSpeed);
            p.velocity = sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed);

            p.totalLifetime = randomRange(minLifetime, maxLifetime);
            p.lifetime = p.totalLifetime;

            sf::Color color;
            if (type == ExplosionType::SmallHit)
            {
                // [تصحيح] استخدام std::uint8_t
                color = sf::Color(255, static_cast<std::uint8_t>(randomRange(200, 255)), 0, 255);
            }
            else
            {
                color = sf::Color(255, static_cast<std::uint8_t>(randomRange(50, 150)), 0, 255);
            }
            p.shape.setFillColor(color);

            explosion.particles.push_back(p);
        }

        // 2. إنشاء جزيئات الدخان
        if (addSmoke)
        {
            int smokeCount = particleCount / 2;
            for (int i = 0; i < smokeCount; ++i)
            {
                Particle p;
                float radius = randomRange(5.f, 12.f);
                p.shape.setRadius(radius);
                
                // [تصحيح SFML 3.0]
                p.shape.setOrigin({radius, radius});
                p.shape.setPosition(position);

                float angle = randomRange(-3.14f / 2.f - 0.5f, -3.14f / 2.f + 0.5f);
                float speed = randomRange(30.f, 80.f);
                p.velocity = sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed);

                p.totalLifetime = randomRange(1.0f, 2.5f);
                p.lifetime = p.totalLifetime;

                int gray = static_cast<int>(randomRange(50, 100));
                // [تصحيح] تحويل الأنواع بشكل صريح
                p.shape.setFillColor(sf::Color(static_cast<std::uint8_t>(gray), static_cast<std::uint8_t>(gray), static_cast<std::uint8_t>(gray), 150));

                explosion.particles.push_back(p);
            }
        }

        explosions.push_back(explosion);
    }

    // دالة التحديث
    void update(float dt)
    {
        for (auto& explosion : explosions)
        {
            if (!explosion.alive) continue;

            bool anyParticleAlive = false;

            for (auto& p : explosion.particles)
            {
                if (p.lifetime <= 0.f) continue;

                p.lifetime -= dt;
                anyParticleAlive = true;

                p.shape.move(p.velocity * dt);

                float ratio = p.lifetime / p.totalLifetime;
                sf::Color c = p.shape.getFillColor();
                
                // [تصحيح SFML 3.0] استبدال sf::Uint8 بـ std::uint8_t
                c.a = static_cast<std::uint8_t>(255 * ratio);
                p.shape.setFillColor(c);
            }

            explosion.alive = anyParticleAlive;
        }

        explosions.erase(
            std::remove_if(explosions.begin(), explosions.end(),
                [](const Explosion& e) { return !e.alive; }),
            explosions.end());
    }

    void draw(sf::RenderWindow& window)
    {
        for (auto& explosion : explosions)
        {
            for (auto& p : explosion.particles)
            {
                if (p.lifetime > 0.f)
                    window.draw(p.shape);
            }
        }
    }

private:
    std::vector<Explosion> explosions;

    float randomRange(float min, float max)
    {
        return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / (max - min));
    }
};