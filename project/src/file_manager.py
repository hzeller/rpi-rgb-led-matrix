class FileManager:
    def __init__(self):
        self.directory = "../images"
    
    def receive_file(self):
        raise NotImplementedError

    def send_file(self):
        raise NotImplementedError
    
    def delete_file(self):
        raise NotImplementedError