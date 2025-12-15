#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <optional> // مهم لـ SFML 3.0 events

#include "game.h"

// التحقق من التصادم
bool rectIntersects(const sf::FloatRect& a, const sf::FloatRect& b)
{
    return a.position.x < b.position.x + b.size.x &&
           a.position.x + a.size.x > b.position.x &&
           a.position.y < b.position.y + b.size.y &&
           a.position.y + a.size.y > b.position.y;
}

int main()
{
    // في SFML 3.0 VideoMode تأخذ Vector2u، الأقواس {} تقوم بالتحويل تلقائياً
    sf::RenderWindow window(sf::VideoMode({700, 500}), "My First SFML Game!");
    window.setFramerateLimit(60);

    ExplosionSystem explosionSystem;

    sf::Texture background;
    if (!background.loadFromFile("background_mountains.png"))
    {
        std::cout << "Error loading background_mountains.png" << std::endl;
    }
    sf::Sprite backgroundSprite(background);
    // setScale تتطلب Vector2f في SFML 3.0
    backgroundSprite.setScale({700.f / background.getSize().x, 500.f / background.getSize().y});

    sf::Texture player;
    if (!player.loadFromFile("player.png"))
    {
        std::cout << "Error loading player.png" << std::endl;
    }
    sf::Sprite playerSprite(player);
    playerSprite.setPosition({350.f, 450.f});
    playerSprite.setScale({0.08f, 0.08f});

    sf::Texture enemyTexture;
    if (!enemyTexture.loadFromFile("enemy.png"))
    {
        std::cout << "Error loading enemy.png" << std::endl;
    }

    sf::Music bgMusic;
    if (!bgMusic.openFromFile("play.wav"))
    {
        std::cout << "Error loading play.wav" << std::endl;
    }
    bgMusic.setLooping(true);
    bgMusic.setVolume(50);
    bgMusic.play();

    sf::SoundBuffer shootBuffer;
    if (!shootBuffer.loadFromFile("shoot.wav"))
    {
        std::cout << "Error loading shoot.wav" << std::endl;
    }
    sf::Sound shootSound(shootBuffer);

    sf::SoundBuffer explosionBuffer;
    if (!explosionBuffer.loadFromFile("explosion.wav"))
    {
        std::cout << "Error loading explosion.wav" << std::endl;
    }
    sf::Sound explosionSound(explosionBuffer);

    float speed = 200.f;
    std::vector<sf::CircleShape> bullets;
    float shootTimer = 0.f;

    std::vector<sf::Sprite> enemies;
    std::vector<float> enemySpeeds;
    const int numEnemies = 5;
    std::srand(static_cast<unsigned int>(std::time(0)));

    auto resetEnemy = [&](size_t index) {
        float startX = 50 + std::rand() % 650;
        float startY = -1 * (std::rand() % 300);
        enemies[index].setPosition({startX, startY});
        float enemySpeed = (static_cast<float>(std::rand()) /
            static_cast<float>(RAND_MAX)) * 100.f - 80.f;
        enemySpeeds[index] = enemySpeed;
    };

    for (int i = 0; i < numEnemies; ++i)
    {
        sf::Sprite enemySprite(enemyTexture);
        enemySprite.setScale({0.06f, 0.06f});
        enemies.push_back(enemySprite);
        enemySpeeds.push_back(0.f);
        resetEnemy(i);
    }

    sf::Clock clock;

    while (window.isOpen())
    {
        float dt = clock.restart().asSeconds();
        shootTimer += dt;

        explosionSystem.update(dt);

        // طريقة SFML 3.0 الجديدة لمعالجة الأحداث
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
        }

        sf::Vector2f movement(0.f, 0.f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) movement.x -= speed * dt;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) movement.x += speed * dt;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) movement.y -= speed * dt;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) movement.y += speed * dt;
        playerSprite.move(movement);

        sf::FloatRect playerBounds = playerSprite.getGlobalBounds();
        sf::Vector2f pos = playerSprite.getPosition();
        if (pos.x < 0) pos.x = 0;
        if (pos.y < 0) pos.y = 0;
        if (pos.x + playerBounds.size.x > 700) pos.x = 700 - playerBounds.size.x;
        if (pos.y + playerBounds.size.y > 500) pos.y = 500 - playerBounds.size.y;
        playerSprite.setPosition(pos);

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) && shootTimer >= 0.15f)
        {
            sf::CircleShape bullet(5.f);
            bullet.setFillColor(sf::Color::Red);
            float bulletX = playerBounds.position.x + (playerBounds.size.x / 2) - 5.f;
            float bulletY = playerBounds.position.y;
            bullet.setPosition({bulletX, bulletY});
            bullets.push_back(bullet);
            shootSound.play();
            shootTimer = 0.f;
        }

        for (size_t i = 0; i < bullets.size(); i++)
        {
            bullets[i].move({0.f, -500.f * dt});
            if (bullets[i].getPosition().y < -10)
            {
                bullets.erase(bullets.begin() + i);
                i--;
            }
        }

        for (size_t i = 0; i < enemies.size(); i++)
        {
            enemies[i].move({0.f, speed * dt});
            enemies[i].move({enemySpeeds[i] * dt, 0.f});
            sf::FloatRect enemyBounds = enemies[i].getGlobalBounds();
            sf::Vector2f enemyPos = enemies[i].getPosition();

            if (enemyPos.y > 600) resetEnemy(i);

            if (enemyPos.x < -enemyBounds.size.x)
            {
                enemyPos.x = 700.f;
                enemies[i].setPosition(enemyPos);
            }
            else if (enemyPos.x > 700.f)
            {
                enemyPos.x = -enemyBounds.size.x;
                enemies[i].setPosition(enemyPos);
            }
        }

        for (size_t i = 0; i < enemies.size(); ++i)
        {
            sf::FloatRect enemyBounds = enemies[i].getGlobalBounds();
            for (size_t j = 0; j < bullets.size(); ++j)
            {
                sf::FloatRect bulletBounds = bullets[j].getGlobalBounds();
                if (rectIntersects(enemyBounds, bulletBounds))
                {
                    explosionSystem.addExplosion(enemies[i].getPosition(), ExplosionType::SmallHit);
                    explosionSound.play();
                    resetEnemy(i);
                    bullets.erase(bullets.begin() + j);
                    break;
                }
            }
        }

        sf::FloatRect currentPlayerBounds = playerSprite.getGlobalBounds();
        for (size_t i = 0; i < enemies.size(); ++i)
        {
            sf::FloatRect enemyBounds = enemies[i].getGlobalBounds();
            if (rectIntersects(currentPlayerBounds, enemyBounds))
            {
                explosionSystem.addExplosion(playerSprite.getPosition(), ExplosionType::BigExplosion);
                explosionSound.play();
                playerSprite.setPosition({350.f, 450.f});
                resetEnemy(i);
            }
        }

        window.clear(sf::Color(50, 50, 50));
        window.draw(backgroundSprite);
        window.draw(playerSprite);
        for (const auto& bullet : bullets) window.draw(bullet);
        for (const auto& enemy : enemies) window.draw(enemy);
        explosionSystem.draw(window);
        window.display();
    }
    return 0;
}