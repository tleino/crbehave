Scenario: Constructing the PPM header
  Given c is canvas(5, 3)
  When ppm is canvas_to_ppm(c)
  Then lines 1-3 of ppm are
    """
    P3
    5 3
    255
    """
