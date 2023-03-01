#!/usr/bin/python

# Color converters
# rgb <--> xyz
# xyz <--> lab
# lab <--> lch
#
# Sources
# * http://www.brucelindbloom.com/index.html?Eqn_XYZ_to_Lab.html
# * http://www.brucelindbloom.com/index.html?Eqn_Lab_to_LCH.html
# * https://de.wikipedia.org/wiki/Lab-Farbraum
# * https://www.easyrgb.com/en/math.php
# * https://www.easyrgb.com/en/convert.php#inputFORM


import math


def rgbToXyz(rgb_in):
  """
  Convert rgb (values 0 to 255) to xyz (values from 0 to 1)
  """
  rgb = [i / 255 for i in rgb_in]

  r = rgb[0]
  g = rgb[1]
  b = rgb[2]

  # Convert sRGB values to linear RGB values
  if r > 0.04045:
      r = ((r + 0.055) / 1.055) ** 2.4
  else:
      r = r / 12.92
  if g > 0.04045:
      g = ((g + 0.055) / 1.055) ** 2.4
  else:
      g = g / 12.92
  if b > 0.04045:
      b = ((b + 0.055) / 1.055) ** 2.4
  else:
      b = b / 12.92

  # Convert linear RGB values to XYZ values
  x = r * 0.4124 + g * 0.3576 + b * 0.1805
  y = r * 0.2126 + g * 0.7152 + b * 0.0722
  z = r * 0.0193 + g * 0.1192 + b * 0.9505

  return [x, y, z]


def xyzToRgb(xyz):
  """
  Convert xyz (values from 0 to 1) to rgb (values from 0 to 255)
  """
  r = xyz[0] *  3.2406 + xyz[1] * -1.5372 + xyz[2] * -0.4986
  g = xyz[0] * -0.9689 + xyz[1] *  1.8758 + xyz[2] *  0.0415
  b = xyz[0] *  0.0557 + xyz[1] * -0.2040 + xyz[2] *  1.0570

  r = r if r > 0 else 0
  g = g if g > 0 else 0
  b = b if b > 0 else 0

  r = 1 if r > 1 else r
  g = 1 if g > 1 else g
  b = 1 if b > 1 else b

  if r > 0.0031308:
    r = 1.055 * ( r ** ( 1 / 2.4 ) ) - 0.055
  else:
    r = 12.92 * r

  if g > 0.0031308:
    g = 1.055 * ( g ** ( 1 / 2.4 ) ) - 0.055
  else:
    g = 12.92 * g

  if b > 0.0031308:
    b = 1.055 * ( b ** ( 1 / 2.4 ) ) - 0.055
  else:
    b = 12.92 * b

  r = round(r * 255)
  g = round(g * 255)
  b = round(b * 255)
  return [r, g, b]


def getBounds():
  """
  Get boundaries for small values for conversion
  of xyz <--> lab
  """
  epsilon = 216/24389
  kappa = 24389/27
  return [epsilon, kappa]


def xyz_fcn(rel_col):
  """
  Calculate result for the normalized input color
  (color/color_ref).
  """
  epsilon, kappa = getBounds()
  if rel_col > epsilon:
    return math.pow(rel_col, 1/3)
  return (kappa * rel_col + 16)/116


def xyz_fcn_inv(rel_col):
  epsilon, kappa = getBounds()
  col_pow = math.pow(rel_col, 3)
  if col_pow > epsilon:
    return col_pow
  return ((rel_col * 116) - 16) / kappa


def getRefXyz():
  """
  Reference white after D65 and viewing angle of 10 degrees (CIE 1964)
  """
  xn_10 = 94.881/100
  yn_10 = 100/100
  zn_10 = 107.304/100
  return (xn_10, yn_10, zn_10)


def xyzToLab(xyz):
  """
  Convert xyz (values from 0 to 1) to lab values.
  L, a, b in percent
  """

  xn_10, yn_10, zn_10 = getRefXyz()

  # Normalize values
  x_rel = xyz[0]/xn_10
  y_rel = xyz[1]/yn_10
  z_rel = xyz[2]/zn_10

  L = 116 * xyz_fcn(y_rel) - 16
  a = 500 * (xyz_fcn(x_rel) - xyz_fcn(y_rel))
  b = 200 * (xyz_fcn(y_rel) - xyz_fcn(z_rel))
  return [L, a, b]


def labToXyz(lab):
  """
  Convert lab (percent) to xyz (values from 0 to 1).
  """
  y_tmp = (lab[0] + 16) / 116
  x_tmp = lab[1] / 500 + y_tmp
  z_tmp = y_tmp - lab[2] / 200

  xn_10, yn_10, zn_10 = getRefXyz()

  x = xyz_fcn_inv(x_tmp) * xn_10
  y = xyz_fcn_inv(y_tmp) * yn_10
  z = xyz_fcn_inv(z_tmp) * zn_10
  return [x, y, z]


def labToLch(lab):
  """
  Convert lab (percent) to lch values.
  lch is a representation of lab in polar coordinates
  (l is equal in both systems), h is the angle in radians
  """
  L = lab[0]
  c = math.sqrt(math.pow(lab[1], 2) + math.pow(lab[2], 2))
  h = math.atan2(lab[2], lab[1])
  if h < 0:
    h += (2 * math.pi)
  return [L, c, h]


def lchToLab(lch):
  """
  Convert lch (h in radians) to lab (percent)
  """
  L = lch[0]
  a = math.cos(lch[2]) * lch[1]
  b = math.sin(lch[2]) * lch[1]
  return [L, a, b]


def print_rgb(rgb):
  print(f"r: {rgb[0]} g: {rgb[1]} b: {rgb[2]}")


def print_xyz(xyz):
  print(f"x: {xyz[0]} y: {xyz[1]} z: {xyz[2]}")


def print_lab(lab):
  print(f"l: {lab[0]} a: {lab[1]} b: {lab[2]}")


def print_lch(lch):
  print(f"l: {lch[0]} c: {lch[1]} h: {lch[2]}")


def rgbToLch(rgb):
  return labToLch(xyzToLab(rgbToXyz(rgb)))

def lchToRgb(lch):
  return xyzToRgb(labToXyz(lchToLab(lch)))

def lchInterpolate(color1, color2, t):
  # Interpolate L*, C, and h values separately
  l1, c1, h1 = color1
  l2, c2, h2 = color2
  l = l1 + t * (l2 - l1)
  c = c1 + t * (c2 - c1)

  h1 = h1 * 180 / math.pi
  h2 = h2 * 180 / math.pi

  # Handle hue interpolation when crossing the 0-360 boundary
  if abs(h2 - h1) <= 180:
    h = h1 + t * (h2 - h1)
  elif h2 > h1:
    h = h1 + t * ((h2 - h1) - 360)
    if h < 0:
      h += 360
  else:
    h = h1 + t * ((h2 - h1) + 360)
    if h >= 360:
      h -= 360

  h *= (math.pi / 180)

  # Convert back to RGB using the interpolated L*, C, and h values
  print((l, c, h))
  return lchToRgb((l, c, h))

def main():
  rgb = [255, 0, 0]
  print_rgb(rgb)
  xyz = rgbToXyz(rgb)
  print_rgb(xyzToRgb(xyz))
  print_xyz(xyz)
  lab = xyzToLab(xyz)
  print_xyz(labToXyz(lab))
  print_lab(lab)
  lch = labToLch(lab)
  print_lab(lchToLab(lch))
  print_lch(lch)
  print(rgbToLch(rgb))
  print(lchToRgb(lch))

  print("\n============================\n")

  rgb = [0, 0, 255]
  print_rgb(rgb)
  xyz = rgbToXyz(rgb)
  print_rgb(xyzToRgb(xyz))
  print_xyz(xyz)
  lab = xyzToLab(xyz)
  print_xyz(labToXyz(lab))
  print_lab(lab)
  lch = labToLch(lab)
  print_lab(lchToLab(lch))
  print_lch(lch)

  red = [255, 0, 0]
  blue = [0, 0, 255]

  for i in range(11):
    t = i/10
    col_int = lchInterpolate(rgbToLch(red), rgbToLch(blue), t)
    print(t)
    print(col_int)

if __name__ == "__main__":
  main()
