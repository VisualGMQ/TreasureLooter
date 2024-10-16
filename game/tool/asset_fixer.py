import xml
import json
import xml.sax
import pathlib
import os

imagePaths = {}
scenePaths = {}


class AssetParseHandler(xml.sax.ContentHandler):
    def __init__(self):
        self.is_texutre_node = False

    def startElement(self, tag, attributes):
        if tag == "texture":
            self.is_texutre_node = True

        if not self.is_texutre_node:
            return

        if not tag == "element":
            return

        path = attributes["path"]
        if not path:
            return

        imagePaths[pathlib.Path("./assets/image").joinpath(path)] = path

    def endElement(self, tag):
        if tag == "texture":
            self.is_texutre_node = False

    def characters(self, content):
        pass

def convertTilemapImagePath(filename):
    fp = open(filename)
    if not fp:
        return

    content: dict = json.load(fp)
    tilesets = content["tilesets"]
    if not tilesets:
        return

    for tileset in tilesets:
        image_path = tileset["image"]
        if not image_path:
            continue
        parentPath = pathlib.Path(filename).parent
        global_path = parentPath.joinpath(pathlib.Path(image_path))

        for k in imagePaths.keys():
            try:
                if global_path.samefile(k):
                    tileset["image"] = imagePaths[k]
            except FileNotFoundError:
                pass

    fp.close()
    fp = open(filename, "w+")
    json.dump(content, fp, indent=4)


def processAllTilemaps(dirname: str):
    for dirpath, dirnames, filenames in os.walk(dirname):
        for filename in filenames:
            path = pathlib.Path(dirpath).joinpath(filename)
            print("modifying ", path, " ...")
            convertTilemapImagePath(path)


if __name__ == "__main__":
    xml.sax.parse("./assets/assets.xml", AssetParseHandler())
    processAllTilemaps("./assets/tilemap")
