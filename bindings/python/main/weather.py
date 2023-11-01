import asyncio
import os

# import the module
import python_weather

async def getweather():
  # declare the client. the measuring unit used defaults to the metric system (celcius, km/h, etc.)
  async with python_weather.Client(unit=python_weather.METRIC) as client:
    # fetch a weather forecast from a city
    weather = await client.get('Barcelona')

    # print(weather.current.description)
    # print(weather.current.temperature)
    print(weather.current.kind)
    #print(str(weather.current))

    # SUNNY = 113
    # PARTLY_CLOUDY = 116
    # CLOUDY = 119
    # VERY_CLOUDY = 122
    # FOG = 143
    # LIGHT_SHOWERS = 176
    # LIGHT_SLEET_SHOWERS = 179
    # LIGHT_SLEET = 182
    # THUNDERY_SHOWERS = 200
    # LIGHT_SNOW = 227
    # HEAVY_SNOW = 230
    # LIGHT_RAIN = 266
    # HEAVY_SHOWERS = 299
    # HEAVY_RAIN = 302
    # LIGHT_SNOW_SHOWERS = 323
    # HEAVY_SNOW_SHOWERS = 335
    # THUNDERY_HEAVY_RAIN = 389
    # THUNDERY_SNOW_SHOWERS = 392
    # returns the current day's forecast temperature (int)


    # get the weather forecast for a few days
    # for forecast in weather.forecasts:
    #   print(forecast)

    #   # hourly forecasts
    #   for hourly in forecast.hourly:
    #     print(f' --> {hourly!r}')

if __name__ == '__main__':
  # see https://stackoverflow.com/questions/45600579/asyncio-event-loop-is-closed-when-getting-loop
  # for more details
  print(os.name)
  if os.name == 'nt':
    asyncio.set_event_loop_policy(asyncio.WindowsSelectorEventLoopPolicy())

  asyncio.run(getweather())

