from pathlib import Path
import uuid


class ImageDataGenerator:
    def __init__(self):
        self.uuid = str(uuid.uuid4())

    def get_uuid(self) -> str:
        return self.uuid

    def generate_single_entry(self, filename: str, location: str, duration: int = 10, visible: bool = True) -> dict:
        return {
            self.uuid: {
                "name": filename,
                "location": location,
                "duration": duration,
                "visible": visible,
            }
        }


class InitImageDataGenerator(ImageDataGenerator):
    def __init__(self) -> None:
        super().__init__()

    def generate_init_images_json(self, path: Path) -> dict:
        init_images = {}
        order = []
        for image in path.iterdir():
            id = str(uuid.uuid4())
            single_entry = self.generate_single_entry(image.name, str(path))
            init_images[id] = single_entry[self.uuid]
            order.append(id)
        return {'images': init_images, 'order': order}
