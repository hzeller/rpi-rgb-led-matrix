import random

class Cities():
    def get_random_one(self):
        cities = [
            "Barcelona", "Rosario"
        ]

        pickup_position = random.randint(0,len(cities)-1)
        return cities[pickup_position]

