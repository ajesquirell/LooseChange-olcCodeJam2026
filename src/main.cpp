// Define OLC_PGE3_APPLICATION to include the implementation of
// the Pixel Game Engine as part of this translation unit
#include <numeric>
#include <string>
#define OLC_PGE3_APPLICATION
#include "olcPixelGameEngine3.h"

#define OLC_PGEX3_MINIAUDIO
#include "olcPGEX3_Miniaudio.h"

#include "utilities/olcUTIL3_Geometry2D.h"

#include <format>
#include <random>

class Animator
{
public:
    Animator() = default;
    void LoadFromFile(PixelGameEngine* pge,
                      std::string sSpriteSheetFileName,
                      int nFrames,
                      float fFramePxSize,
                      float fTimePerFrame)
    {
        pge->CreateImageFromFile(spriteSheet, sSpriteSheetFileName);
        this->nFrames = nFrames;
        this->fFramePxSize = fFramePxSize;
        this->fTimePerFrame = fTimePerFrame;
    }
    void Update(float fElapsedTime)
    {
        fTimeCounter += fElapsedTime;
        if (fTimeCounter >= fTimePerFrame) {
            fTimeCounter -= fTimePerFrame;
            nCurrentFrame++;
            if (nCurrentFrame >= nFrames) {
                nCurrentFrame = 0;
            }
        }
    }
    olc::ImageRegion GetFrame()
    {
        return spriteSheet.region({ nCurrentFrame * fFramePxSize, 0.0f }, { fFramePxSize, fFramePxSize });
    }
    olc::ImageRegion GetStaticFrame()
    {
        // Just use first frame
        return spriteSheet.region({ 0.0f, 0.0f }, { fFramePxSize, fFramePxSize });
    }

private:
    float fTimeCounter = 0;
    int nCurrentFrame;

    olc::Image spriteSheet;
    int nFrames;
    float fFramePxSize;
    float fTimePerFrame;
};

class LooseChangeEngine : public olc::PixelGameEngine
{
public:
    LooseChangeEngine()
    {
        sAppName = "Loose Change - olc::CodeJam 2026";
        if (!InstallSystemExtension(&audio)) throw std::runtime_error("Failed to install olcPGEX3_Miniaudio");
    }

public:
    // Called once at the start, so create things here
    bool OnUserCreate() override
    {
        // load `assets/song1.mp3` into `song1`
        audio.CreateSoundFromFile(song1, "assets/song1.mp3");

        /**
         * this is here to demonstrate how the adventurous can
         * exploit other features of miniaudio that hasn't been
         * abstracted by the PGEX
         *
         * Here you get a pointer to a next active voice, or the
         * currently playing voice.
         */
        ma_sound_set_position(song1.GetMASound(), 0.0f, 0.0f, 0.0f);

        // load `assets/SampleA.wav` into `sample`
        audio.CreateSoundFromFile(sample, "assets/SampleA.wav");

        // create all of the waveforms at 0.1 amplitude at 440Mhz (A4)
        audio.CreateWaveform(sine, olc::ext::Miniaudio::Waveform::Type::Sine, 0.1, 440.0);
        audio.CreateWaveform(square, olc::ext::Miniaudio::Waveform::Type::Square, 0.1, 440.0);
        audio.CreateWaveform(triangle, olc::ext::Miniaudio::Waveform::Type::Triangle, 0.1, 440.0);
        audio.CreateWaveform(sawtooth, olc::ext::Miniaudio::Waveform::Type::Sawtooth, 0.1, 440.0);

        CreateImageFromFile(background, "assets/repeated-square.png");
        bronzeCoin.LoadFromFile(this, "assets/coins/bronze.png", 5, coinPxSize, 0.1);
        silverCoin.LoadFromFile(this, "assets/coins/silver.png", 5, coinPxSize, 0.1);
        goldCoin.LoadFromFile(this, "assets/coins/gold.png", 5, coinPxSize, 0.1);
        gemCoin.LoadFromFile(this, "assets/coins/gem.png", 4, coinPxSize, 0.1);

        playerPos = {
            ScreenSize().x - ScreenSize().x / 6.0f, ScreenSize().y / 2.0f
        }; // TODO middle of board, and is middle always safe? OR always starts as safe?
        playerSize = { 5, 5 };

        boardPosStart.x = 0;
        boardPosStart.y = ScreenSize().y / 5.0f;
        boardPosEnd.x = ScreenSize().x - ScreenSize().x / 4.0f; // TODO Leaves room on right for exCHANGEr
        boardPosEnd.y = ScreenSize().y;
        boardTimer = TIMER_DURATION_NORMAL;
        boardState = NORMAL;

        olc::vf2d board_size{ boardPosEnd.x - boardPosStart.x, boardPosEnd.y - boardPosStart.y };

        tileSize = { board_size.x / board.size.x, board_size.y / board.size.y };

        // TODO Either randomize coins or start in corners
        coins[0].tileIndex = 0;
        coins[1].tileIndex = board.size.x - 1;
        coins[2].tileIndex = (board.size.y - 1) * board.size.x;
        coins[3].tileIndex = (board.size.y - 1) * board.size.x + board.size.x - 1;

        return true;
    }

    // Called every frame, so update things here
    bool OnUserUpdate(float fElapsedTime) override
    {
        // // toggle background playback
        // if (keyboard.GetKey(olc::Key::K1).bPressed) {
        //   backgroundPlay = !backgroundPlay;
        //   if (backgroundPlay)
        //     audio.EnableBackgroundPlayback();
        //   else
        //     audio.DisableBackgroundPlayback();
        // }

        // panning
        // if (keyboard.GetKey(olc::Key::MINUS).bHeld)
        //   pan -= 1.0f * fElapsedTime;
        //
        // if (keyboard.GetKey(olc::Key::EQUALS).bHeld)
        //   pan += 1.0f * fElapsedTime;
        //
        // // pitch
        // if (keyboard.GetKey(olc::Key::OEM_4).bHeld)
        //   pitch -= 1.0f * fElapsedTime;
        //
        // if (keyboard.GetKey(olc::Key::OEM_6).bHeld)
        //   pitch += 1.0f * fElapsedTime;
        //
        // // volume
        // if (keyboard.GetKey(olc::Key::DOWN).bHeld)
        //   volume -= 1.0f * fElapsedTime;
        //
        // if (keyboard.GetKey(olc::Key::UP).bHeld)
        //   volume += 1.0f * fElapsedTime;
        //
        // // distance
        // if (keyboard.GetKey(olc::Key::LEFT).bHeld)
        //   distance -= 10.0f * fElapsedTime;
        //
        // if (keyboard.GetKey(olc::Key::RIGHT).bHeld)
        //   distance += 10.0f * fElapsedTime;

        // panning
        // pan = std::clamp(pan, -1.0f, 1.0f);
        // song1.SetPan(pan);
        //
        // // pitch
        // pitch = std::clamp(pitch, 0.0f, 2.0f);
        // song1.SetPitch(pitch);
        //
        // // volume
        // volume = std::clamp(volume, 0.0f, 1.0f);
        // song1.SetVolume(volume);

        // this is here to demosntrate how the adventurous can exploit other
        // features of miniaudio that haven't been abstracted by the PGEX.
        // distance = std::clamp(distance, 0.0f, 100.0f);
        // ma_engine_listener_set_position(&audio.GetEngine(), 0, 0.0f, distance,
        //                                 0.0f);

        if (keyboard.GetKey(olc::Key::UP).bHeld) {
            playerPos.y -= playerSpeed * fElapsedTime;
        }

        if (keyboard.GetKey(olc::Key::DOWN).bHeld) {
            playerPos.y += playerSpeed * fElapsedTime;
        }

        if (keyboard.GetKey(olc::Key::LEFT).bHeld) {
            playerPos.x -= playerSpeed * fElapsedTime;
        }

        if (keyboard.GetKey(olc::Key::RIGHT).bHeld) {
            playerPos.x += playerSpeed * fElapsedTime;
        }

        // TODO fix speed when both directions pressed

        if (playerPos.x < boardPosStart.x + playerSize.x) playerPos.x = boardPosStart.x + playerSize.x;
        if (playerPos.x > ScreenSize().x - playerSize.x) playerPos.x = ScreenSize().x - playerSize.x;
        if (playerPos.y < boardPosStart.y + playerSize.y) playerPos.y = boardPosStart.y + playerSize.y;
        if (playerPos.y > ScreenSize().y - playerSize.y) playerPos.y = ScreenSize().y - playerSize.y;

        // Player collisions with coins
        utils::geom2d::circle<float> playerCircle(playerPos,
                                                  playerSize.x); // TODO change to rect intersection with a real sprite
        for (Coin& coin : coins) {
            olc::vf2d coinPos = coin.GetPos(*this);
            utils::geom2d::rect<float> coinRect({ coinPos.x, coinPos.y }, { coinPxSize, coinPxSize });

            if (utils::geom2d::overlaps(playerCircle, coinRect)) {
                // TODO collect coin. Show counter in top bar or above player head
                coin.isAcquired = true;
            }
        }

        bool playerInTheRed = false;
        if (utils::geom2d::contains(utils::geom2d::rect<float>(boardPosStart, boardPosEnd), playerPos)) {
            // Player inside board
            int xIndex = (playerPos.x - boardPosStart.x) / tileSize.x;
            int yIndex = (playerPos.y - boardPosStart.y) / tileSize.y;
            int playerTileIdx = yIndex * board.size.x + xIndex;
            if (playerTileIdx >= 0 && !board.tiles[playerTileIdx]) {
                playerInTheRed = true;
            }
        }

        // Update Coins
        for (Coin& coin : coins) {
            if (coin.isAcquired) {
                coin.heldTime += fElapsedTime;

                float inflationRate = baseInflationRate;
                if (playerInTheRed) {
                    // ...MASSIVE INFLATION!!!
                    inflationRate *= 3;
                }

                // TODO normal percentage might be enough of a challenge for now...
                // for (int i = 1; i <= PENALTY_INCREMENT_LIMIT; i++) {
                //     if (coin.heldTime > HELD_TIME_PENALTY * i) {
                //         inflationRate += baseInflationRate;
                //     }
                // }

                // Inflation is happening to decrease value!
                coin.value -= (coin.value * inflationRate * fElapsedTime);
                if (coin.value < 0) coin.value = 0;
            }
        }

        // Update board
        boardTimer -= fElapsedTime;
        if (boardTimer < 0) {
            if (boardState == NORMAL) {
                boardTimer = TIMER_DURATION_TRANSITIONING;
                boardState = TRANSITIONING;
            } else if (boardState == TRANSITIONING) {
                boardTimer = TIMER_DURATION_UPDATING;
                boardState = UPDATING;
                for (int i = 0; i < board.tiles.size(); i++) {
                    static auto gen = std::bind(
                        std::uniform_int_distribution<>(0, 1),
                        std::default_random_engine()); // TODO need to be more aggressive as time goes on, or as values get higher
                    board.tiles[i] = gen();
                }
            } else {
                boardTimer = TIMER_DURATION_NORMAL;
                boardState = NORMAL;
            }
        }

        /* === DRAWING === */

        draw.Clear(olc::Colour::VERY_DARK_BLUE);
        draw.ImageRect(background, { 0, boardPosStart.y }, ScreenSize());
        draw.FilledRect({ 0, boardPosStart.y - 5 }, { (float)ScreenSize().x, 5 }, olc::Colour::DARK_MAGENTA);

        // Update coin animations
        bronzeCoin.Update(fElapsedTime);
        silverCoin.Update(fElapsedTime);
        goldCoin.Update(fElapsedTime);
        gemCoin.Update(fElapsedTime);

        // Draw Board
        for (int x = 0; x < board.size.x; x++) {
            for (int y = 0; y < board.size.y; y++) {
                olc::vf2d pos = GetTilePos(x, y);
                std::size_t tile_index = y * board.size.x + x;
                olc::Pixel color = board.tiles[tile_index] ? olc::Colour::BLUE : olc::Colour::RED;

                switch (boardState) {
                case TRANSITIONING: {
                    // TODO COLORS could change, and the current good/bad shown on the top
                    float t = boardTimer / TIMER_DURATION_TRANSITIONING;
                    color.a = t * 255;
                    draw.FilledRect(pos, tileSize, color);

                } break;

                case UPDATING: {
                    // Animate the new color in like a pulsing way
                    // float t = std::cos(boardTimer / TIMER_DURATION_UPDATING * 3 * M_PI / 2);
                    // t = std::abs(t);// * (1.0f - boardTimer / TIMER_DURATION_UPDATING);
                    // uint8_t alpha_multiplier = boardTimer / TIMER_DURATION_UPDATING > 0.5 ? 128 : 255;
                    // color.a = t * alpha_multiplier;

                    float t = 1.f - boardTimer / TIMER_DURATION_UPDATING;
                    color.a = t * 255;
                    // Epilepsy mode...
                    // if (std::fmod(t, 0.05) < 0.025) color.a = 0;
                    draw.FilledRect(pos, tileSize, color);
                } break;

                case NORMAL: {
                    draw.FilledRect(pos, tileSize, color);

                } break;
                }
                draw.Rect(pos, tileSize, olc::Colour::BLACK);
            }
        }

        // Draw Coins
        for (const Coin& coin : coins) {
            // TODO make a sprite for coins that can be colored
            if (!coin.isAcquired) {
                draw.Image(coin.GetImage(*this), coin.GetPos(*this));
            }
        }

        // Draw Player
        draw.FilledCircle(playerPos, 5, olc::Colour::GREEN);

        // Draw top info
        static float pulsingTimer = 0;
        if (playerInTheRed) {
            // draw.FilledRect({ScreenSize().x / 3.0f, 0}, {ScreenSize().x / 3.0f, boardPosStart.y - 5}, olc::Colour::RED);

            // Box shadow around screen, show "MEGA INFLATION" somewhere?
            if (std::accumulate(
                    coins.begin(), coins.end(), 0, [](int acc, const Coin& coin) { return acc + coin.isAcquired; }))
            {
                float width = ScreenSize().x;
                float height = ScreenSize().y;
                olc::Pixel col = olc::Colour::RED;
                olc::Pixel blank = olc::Colour::BLANK;

                pulsingTimer += fElapsedTime;
                float vignetteSize = 30.0f + std::cos(pulsingTimer * 2 * M_PI) * -10;

                draw.FilledRect({ 0, 0 }, { width, vignetteSize }, col, col, blank, blank);
                draw.FilledRect({ 0, height - vignetteSize }, { width, vignetteSize }, blank, blank, col, col);
                draw.FilledRect({ 0, 0 }, { vignetteSize, height }, col, blank, col, blank);
                draw.FilledRect({ width - vignetteSize, 0 }, { vignetteSize, height }, blank, col, blank, col);
            }
        } else {
            pulsingTimer = 0;
        }

        float totalScore = 0;
        for (int i = 0; i < coins.size(); i++) {
            const Coin& coin = coins[i];
            constexpr float scale = 1.5f;
            constexpr float distBtwCoins = 50.0f;
            olc::vf2d pos{ ScreenSize().x / 2.0f - (coinPxSize * 2) - (distBtwCoins * 1.5f),
                           boardPosStart.y / 2.0f - (coinPxSize * scale) / 2.0f - 10.0f };
            pos.x += (coinPxSize + distBtwCoins) * i;
            draw.Image(coin.GetStaticImage(*this), pos, { scale, scale });

            pos.x -= 15;
            pos.y += coinPxSize + 15;
            draw.String(pos, std::format("{:^7}", "$" + std::format("{:.2f}", coin.value)));

            totalScore += coin.value;
        }
        draw.String({ ScreenSize().x - 110.0f, 10 }, "TOTAL:", olc::Colour::WHITE, { 1.5, 1.5 });
        draw.String({ ScreenSize().x - 150.0f, 35 },
                    std::format("{:>7}", "$" + std::format("{:.2f}", totalScore)),
                    olc::Colour::CYAN,
                    { 2.5, 2.5 });

        // DEBUG INFO
        // draw.String({ 5, 5 }, std::to_string(playerTileIdx));

#if OLC_HOST == OLC_HOST_EMSCRIPTEN
        return true;
#else
        return !keyboard.GetKey(olc::Key::ESCAPE).bPressed;
#endif
    }

    // put this here to have access to audio!
    olc::ext::Miniaudio::AudioEngine audio;

private:
    // sounds
    olc::ext::Miniaudio::Sound song1;
    olc::ext::Miniaudio::Sound sample;

    olc::ext::Miniaudio::Waveform sine;
    olc::ext::Miniaudio::Waveform square;
    olc::ext::Miniaudio::Waveform triangle;
    olc::ext::Miniaudio::Waveform sawtooth;

    // For demonstration controls, with sensible default values
    float pan = 0.0f;
    float pitch = 1.0f;
    float volume = 1.0f;
    float distance = 0.0f;

    olc::vf2d playerPos;
    olc::vf2d playerSize;
    const float playerSpeed = 120.0f;

    struct Board
    {
        static constexpr olc::vu2d size{ 5, 5 };
        std::array<bool, size.x * size.y> tiles;
    };

    Board board;
    olc::vf2d boardPosStart;
    olc::vf2d boardPosEnd;
    float boardTimer;
    enum { NORMAL, TRANSITIONING, UPDATING } boardState;

    static constexpr float TIMER_DURATION_NORMAL = 4;
    static constexpr float TIMER_DURATION_TRANSITIONING = 1;
    static constexpr float TIMER_DURATION_UPDATING = 2;

    olc::vf2d tileSize;

    olc::vf2d GetTilePos(int xIndex, int yIndex)
    {
        // - 1 because it will overdraw off the screen
        // TODO maybe give it a small buffer to look better, then
        // we don't need this?
        return { boardPosStart.x + tileSize.x * xIndex, boardPosStart.y + tileSize.y * yIndex - 1 };
    }

    struct Coin
    {
        float value =
            1.0f; // TODO Allow the numbers to be big enough to be fun. Maybe start at silver so bronze is like a penalty. Maybe you hit bronze quick enough after picking up to show user how it works
        uint32_t tileIndex = 0;
        bool isAcquired = false;
        float heldTime = 0.0f;

        // TODO consider having coin positions in float space - might make it more interesting if they are between tiles
        olc::vf2d GetPos(LooseChangeEngine& engine) const
        {
            int xIndex = tileIndex % engine.board.size.x;
            int yIndex = tileIndex / engine.board.size.x;
            olc::vf2d tilePos = engine.GetTilePos(xIndex, yIndex);
            return tilePos + engine.tileSize / 2.f - engine.coinPxSize / 2.f;
        }

        olc::ImageRegion GetImage(LooseChangeEngine& engine) const
        {
            return get_animator(engine).GetFrame();
        }

        olc::ImageRegion GetStaticImage(LooseChangeEngine& engine) const
        {
            return get_animator(engine).GetStaticFrame();
        }

    private:
        Animator& get_animator(LooseChangeEngine& engine) const
        {
            if (value > 10.0f) {
                return engine.gemCoin;
            }
            if (value > 5.0f) {
                return engine.goldCoin;
            }
            if (value > 2.0f) {
                return engine.silverCoin;
            }
            return engine.bronzeCoin;
        }
    };
    std::array<Coin, 4> coins;
    static constexpr int coinPxSize = 16;

    // TODO ORRRRRR stepping on red increases your CURRENT inflation rate for the rest of the game!!!! but then user can never recover....
    static constexpr float baseInflationRate = 0.03f; // Percent per second

    // Static increments where inflation rate increases, per coin (seconds)
    static constexpr float HELD_TIME_PENALTY = 5.0f;
    static constexpr int PENALTY_INCREMENT_LIMIT = 4;

    // Assets
    olc::Image background;
    Animator bronzeCoin;
    Animator silverCoin;
    Animator goldCoin;
    Animator gemCoin;
};

// Main entry point for the application
int main()
{
    // Construct demo application
    LooseChangeEngine game;

    PGEConfig config;
    config.bVSync = false;
    config.vPixelSize = { 2, 2 };
    config.vScreenSize = { 640, 360 };

    if (game.Construct(config)) {
        // Start the application
        game.Start();
    }

    return 0;
}
