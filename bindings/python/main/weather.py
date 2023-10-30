import asyncio
import os

# import the module
import python_weather

async def getweather():
  # declare the client. the measuring unit used defaults to the metric system (celcius, km/h, etc.)
  async with python_weather.Client(unit=python_weather.METRIC) as client:
    # fetch a weather forecast from a city
    weather = await client.get('Barcelona')

    # returns the current day's forecast temperature (int)
    print(weather.current.temperature)

    # get the weather forecast for a few days
    for forecast in weather.forecasts:
      print(forecast)

      # hourly forecasts
      for hourly in forecast.hourly:
        print(f' --> {hourly!r}')

if __name__ == '__main__':
  # see https://stackoverflow.com/questions/45600579/asyncio-event-loop-is-closed-when-getting-loop
  # for more details
  print(os.name)
  if os.name == 'nt':
    asyncio.set_event_loop_policy(asyncio.WindowsSelectorEventLoopPolicy())

  asyncio.run(getweather())
