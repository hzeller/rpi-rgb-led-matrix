import json
import os

class ImageManager:
    def __init__(self):
        dirname = os.path.dirname(__file__)
        filename = os.path.join(dirname, "../resources/init_images.json")
        self.index = 0
        with open(filename) as fs:
            init_json = json.load(fs)
            self.images = init_json["images"]
            self.order = init_json["order"]      

    def add_image(self, image_name: str, duration=10, visible=True) -> None:
        self.image[image_name] = {"duration": duration, "visible": visible}

    def delete_images(self, image_name: str):
        try:
            del self.image[image_name]
        except KeyError:
            pass
        try:
            self.order.remove(image_name)
        except KeyError:
            pass

    def set_visible(self, image_name: str, visible: bool):
        raise NotImplementedError

    def set_order(self, new_order: list) -> None:
        self.order = new_order
    
    def get_order(self):
        raise NotImplementedError

    def update_index(self):
        print(self.images)
        if self.index >= len(self.order):
            self.index = 0
        else:
            self.index += 1



if __name__ == "__main__":
    i = ImageManager()
    i.update_index()