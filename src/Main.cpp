#define EIGEN_USE_THREADS

#include <SFML/Graphics.hpp>
#include <unsupported/Eigen/CXX11/Tensor>
#include "Globals.h"
#include "Convenience.h"
#include "Math.h"
#include "HSVColor.h"

using namespace Eigen;
using namespace std;
using namespace sf;

int main()
{
  // Executive --------------------------------------------------------------------------
  
#ifdef FOURIER
  RenderWindow fft_window(VideoMode(g::width * g::scale, g::height * g::scale), "Frequency Spectrum");
  fft_window.setPosition(sf::Vector2i{int(ceil(g::width * g::scale)), 0});
  Image fft_img; fft_img.create(g::width, g::height, Color::Green);
  Texture fft_tex; fft_tex.create(g::width, g::height); fft_tex.setRepeated(true);
  Sprite fft_spr; fft_spr.setTexture(fft_tex); fft_spr.setScale(5, 5);

  // Zoom in on the origin
  fft_spr.setTextureRect(IntRect(g::width / 2, g::height / 2, g::width, g::height));
  fft_spr.setOrigin(g::width/2 + 0.5, g::height/2 + 0.5); fft_spr.setPosition(g::width / 2, g::height / 2);
#endif

  RenderWindow window(VideoMode(g::width * g::scale, g::height * g::scale), g::title);
  window.setPosition(sf::Vector2i{0, 0});
  Image img; img.create(g::width, g::height, Color::Green);
  Texture tex; tex.create(g::width, g::height);
  Sprite spr; spr.setTexture(tex, true); spr.setScale(g::scale, g::scale);

  ThreadPool tp(g::numThreads);
  ThreadPoolDevice tpd(&tp, g::numThreads);

  // Math -------------------------------------------------------------------------------

#ifdef FREE
  Mat field(g::width + 2, g::height + 2);
#else
  Mat field(g::width, g::height);
#endif
  field.device(tpd) = field.generate(GaussianGenerator<2>(Scals<2>{g::width / 2, g::height / 3}, Scals<2>{50, 50})) * (g::scalar)7;
  //field.device(tpd) = sineMode(22,  6, g::width, g::height)
  //                  + sineMode( 6, 22, g::width, g::height)
  //                  + sineMode(18, 14, g::width, g::height)
  //                  + sineMode(14, 18, g::width, g::height);
  //field.device(tpd) = sineMode(11,  3, g::width, g::height)
  //                  + sineMode( 3, 11, g::width, g::height)
  //                  + sineMode( 9,  7, g::width, g::height)
  //                  + sineMode( 7,  9, g::width, g::height);
  //field.device(tpd) = sineMode(1, 1, g::width, g::height);
  int rad = 6;
  int diam = 2 * rad + 1;
  Mat ker = laplacian();
  //Mat ker = TensorMap<Mat>(0, diam - 2, diam - 2)
  //         .generate(GaussianGenerator<2>(Scals<2>{(g::scalar)(rad - 1.0), (g::scalar)(rad - 1.0)}, Scals<2>{1.0, 1.0}))
  //         .pad(Pairs<2>{Pair{2, 2}, Pair{2, 2}})
  //         .convolve(laplacian(), Dims<2>{0, 1});
  //cout << ker << endl;
  //cout << ker.sum() << endl;
  
#ifdef WAVE
  Mat fielddt(g::width, g::height);
  fielddt.setConstant(0);
#endif

#ifdef FOURIER
  Matplex fft(g::width, g::height);
  //fft.setZero();
  //fft(1, 1) = 300000;
  //fft(1, g::height - 1) = -300000;
  //field = fft.fft<RealPart, FFT_REVERSE>(Dims<2>{0, 1});
#endif

  // IO ----------------------------------------------------------------------------------

  int frame = 0;
  float lastChunk = 0;
  float lastFrame = 0;
  Clock clock;

  bool ldown = false;
  g::scalar intensity = 0.2;
  int prevx = g::width / 2;
  int prevy = g::height / 2;

  while (window.isOpen())
  {
    frame++;
    float thisFrame = clock.getElapsedTime().asSeconds();
    if (thisFrame - lastFrame < g::dt)
    {
      sleep(seconds(g::dt - (thisFrame - lastFrame)));
    }
    lastFrame = clock.getElapsedTime().asSeconds();
    if (frame % 60 == 0)
    {
      cout << "FPS: " << 60 / (thisFrame - lastChunk) << endl;
      lastChunk = thisFrame;
    }

    // Event loop
    sf::Event event;
    while (window.pollEvent(event))
    {
      switch (event.type)
      {
      case Event::Closed:
        window.close();
        break;
      case Event::KeyPressed:
        switch (event.key.code)
        {
        case Keyboard::Escape:
          window.close();
          break;
        case Keyboard::Q:
          intensity = -intensity;
        }
        break;
      case Event::MouseButtonPressed:
      {
        g::scalar x = event.mouseButton.x / g::scale;
        g::scalar y = event.mouseButton.y / g::scale;
        switch (event.mouseButton.button)
        {
        case Mouse::Button::Left:
          ldown = true;
          break;
        case Mouse::Button::Right:
          field.device(tpd) += field.generate(GaussianGenerator<2>(Scals<2>{x, y}, Scals<2>{1, 1})) * (intensity * 50);
          break;
        }
        break;
      }
      case Event::MouseButtonReleased:
        switch (event.mouseButton.button)
        {
        case Mouse::Button::Left:
          ldown = false;
        }
        break;
      case Event::MouseMoved:
      {
        g::scalar x = event.mouseMove.x / g::scale;
        g::scalar y = event.mouseMove.y / g::scale;
        g::scalar dist = Eigen::Vector2d(x - prevx, y - prevy).norm();
        if (ldown && dist > 1) {
          field.device(tpd) += field.generate(GaussianGenerator<2>(Scals<2>{x, y}, Scals<2>{dist * 4, dist * 4})) * intensity;
        }
        prevx = x;
        prevy = y;
        break;
      }
      }
    }

#ifdef FOURIER
    if(frame % 1 == 0)
    {
      fft.device(tpd) = field.fft<BothParts, FFT_FORWARD>(Dims<2>{0, 1});
    }
#endif

    //if (!ldown)
    if (true)
    {
      for (int i = 0; i < g::stepsPerFrame; i++)
      {
#ifdef FREE
        field.slice(Dims<2>{0, 1},             Dims<2>{1, g::height}   ).device(tpd) = field.slice(Dims<2>{1, 1},         Dims<2>{1, g::height});
        field.slice(Dims<2>{g::width + 1, 1},  Dims<2>{1, g::height}   ).device(tpd) = field.slice(Dims<2>{g::width, 1},  Dims<2>{1, g::height});
        field.slice(Dims<2>{0, 0},             Dims<2>{g::width + 2, 1}).device(tpd) = field.slice(Dims<2>{0, 1},         Dims<2>{g::width + 2, 1});
        field.slice(Dims<2>{0, g::height + 1}, Dims<2>{g::width + 2, 1}).device(tpd) = field.slice(Dims<2>{0, g::height}, Dims<2>{g::width + 2, 1});
        auto lap = field.convolve(laplacian(), Dims<2>{0, 1}) * g::dt;
#else
        auto lap = field.pad(Pairs<2>{Pair{1, 1}, Pair{1, 1}}).convolve(ker, Dims<2>{0, 1})* g::dt;
        //auto lap = field.pad(Pairs<2>{Pair{rad, rad}, Pair{rad, rad}}).convolve(ker, Dims<2>{0, 1})* g::dt;
#endif
#ifdef WAVE
#ifdef DAMP
        fielddt.device(tpd) = fielddt * g::dpf;
#endif
 /* Mat tmp(g::width, g::height);
  tmp.setConstant(cos(thisFrame));*/
        fielddt.device(tpd) += lap * g::cc/* + tmp * g::dt*/;
#ifdef FREE

        field.slice(Dims<2>{1, 1}, Dims<2>{g::width, g::height}).device(tpd) += fielddt * g::dt;
#else
        field.device(tpd) += fielddt * g::dt;
#endif
#endif
#ifdef DIFFUSION
        field.slice(Dims<2>{1, 1}, Dims<2>{g::width, g::height}).device(tpd) += (lap * (g::c / 5)).eval();
#endif
#ifdef EFFUSION
        field.slice(Dims<2>{1, 1}, Dims<2>{g::width, g::height}).device(tpd) -= lap.eval();
#endif
      }
    }

    for (int i(0); i < g::width; i++)
    {
      for (int j(0); j < g::height; j++)
      {
        g::scalar r = field(i, j);
        img.setPixel(i, j, Color(clamp<g::scalar>(    r       , 0, 1) * 255,
                                 clamp<g::scalar>(abs(r) - 0.5, 0, 1) * 255,
                                 clamp<g::scalar>(   -r       , 0, 1) * 255));

#ifdef FOURIER
        g::complex c = fft(i, j);
        g::scalar value = 1 / (1 + 1 / abs(c));
        fft_img.setPixel(i, j, HSVColor(arg(c), 1, log(abs(c)) / 4).RGB());
#endif
      }
    }

    tex.update(img);
    window.clear();
    window.draw(spr);
    window.display();

#ifdef FOURIER
    fft_tex.update(fft_img);
    fft_window.clear();
    fft_window.draw(fft_spr);
    fft_window.display();
#endif

    //cin.get();
  }

  return 0;
}