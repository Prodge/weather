##Weather
Returns current weather information in a formattable string.

Written for use with bar using baskerville's panel scripts for bspwm
(https://github.com/baskerville/bspwm/tree/master/examples/panel)

Weather uses the openweathermap.org API http://openweathermap.org/current

###Dependencies
- libcurl4-openssl-dev

###How to:
####Install
- This functionality is currently not availible and will be added to the makefile soon. 
- You can compile it for yourself using 'make'

####Use
- Simply type 'weather' in your prefered terminal
- weather will take the following arguments:
    - -s [sec]: (subscribe) will ping for and reprint your specified weather information every [sec] seconds
    - -c [city]: Specifies the city to get weather information for
    - -C [lat,lon]: Specifies the coordinates to get weather information for (coordinates will overide the city if both are used)
    - -f "[format]": Specifies the output format (see format specifiers below). Anything character that is not preceded by a '%' will be simply printed out as it is
- The following format specifiers can be used:
    - %c: City Name
    - %C: Coordinates
    - %l: Latitude
    - %L: Longitude
    - %n: New line "\n"
    - %u: The URL that the information was scraped from
    - %N: Null byte character '\0'
    - %s: Sunrise time
    - %S: Sunset time
    - %t: Temperature in Kelvin
    - %T: Temperature in Celsius
    - %f: Temperature in Fahrenheit
    - %h: Humidity Percentage
    - %p: Air Pressure
    - %w: Wind Speed
    - %W: Wind Descriptor eg: "Fresh Breeze"
    - %d: Wind Direction abreviated
    - %D: Wind Direction
    - %a: Wind Direction Angle
    - %r: Raining (yes/no)
    - %k: Cloud Coverage Percentage
    - %K: Cloud Coverage Descriptor eg: "clear sky"
    - %i: Weather Descriptor eg: "Sky is Clear"
    - %U: Time the weather information was last updated
    - %%: Will print a '%'
- Examples:
    - *weather -c london -s 100 -f "%c:%t"*
    - _London:283.77_
    - *weather -c london -f "The Sun rose today in %c at %s and will set at %S"*
    - _The Sun rose today in London at 2014-12-17T08:01:29 and will set at 2014-12-17T15:52:10_
    - *weather -C "46,-60" -f "Location: %c  Air Pressure: %phpa  Raining: %r"*
    - _Location: Broughton  Air Pressure:  1019hpa  Raining: no_
    - *weather -C "41,-74" -f "It is %i at %c. Wind is %W in a %D direction"*
    - _It is light rain at Harrington Park. Wind is Calm in a West-southwest direction_

####Use with Bar/Panel
- Coming Soon

####Use with Conky
- Coming Soon

###Todo:
- Create a Makefile with install
- Autodetect location?
- Write a man page
- Create instructions for:
    - Installing
    - Running with baskervilles panel script
    - Running with bar (basic script)
    - Running with conky

###Known Bugs
- Coordinates cannot contain a decimal point
