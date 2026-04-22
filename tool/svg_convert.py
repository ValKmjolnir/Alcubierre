import cairosvg
from PIL import Image
import io

def svg_to_png_resvg(svg_path: str) -> bytes:
    with open(svg_path, 'rb') as f:
        svg_data = f.read()

    png_data = cairosvg.svg2png(svg_data, output_width=1024, output_height=1024, scale=1.0)
    return png_data

def png_to_ico(png_bytes: bytes, ico_path: str):
    sizes = [16, 32, 48, 64, 128, 256, 512, 1024]

    master = Image.open(io.BytesIO(png_bytes))
    if master.mode != 'RGBA':
        master = master.convert('RGBA')

    icon_images = []
    for size in sizes:
        resized = master.resize((size, size), Image.Resampling.LANCZOS)
        icon_images.append(resized)

    icon_images.reverse()
    icon_images[0].save(
        ico_path,
        format='ICO',
        append_images=icon_images[1:]
    )

png_data = svg_to_png_resvg("docs/logo/icon.svg")
image = Image.open(io.BytesIO(png_data))
image.save("docs/logo/icon.png")
png_to_ico(png_data, "docs/logo/icon.ico")