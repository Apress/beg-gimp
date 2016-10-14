;; allmodes.scm: Show all layer modes.
;; Copyright (C) 2005 by Akkana Peck, akkana@shallowsky.com.
;; 
;; This program is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License.

;; All Layer Modes:
;; Starting from an image, make a page showing how the image looks
;; against a fixed background color in each of the layer modes.
;; I recommend starting from a small image with a transparent background.

(define (script-fu-all-layer-modes srcimg srcdrawable bgcolor
                                   font fontsize textcolor)
  (let* ((old-fg-color (car (gimp-context-get-foreground)))
         (imgw (car (gimp-image-width srcimg)))
         (imgh (car (gimp-image-height srcimg)))
         (nx (if (> imgw imgh) 4 5))
         (ny (if (> imgw imgh) 5 4))
         (xgap 10)
         (ygap 30)
         (dx (+ imgw xgap))
         (dy (+ imgh ygap))
         (pagew (* nx dx))
         (pageh (* ny dy))
         (namelist (list "Normal" "Addition"
                    "Subtract" "Difference"
                    "Multiply" "Divide"
                    "Dodge" "Burn" "Screen" "Overlay"
                    "Hard Light" "Soft Light"
                    "Darken Only" "Lighten Only"
                    "Grain extract" "Grain merge"
                    "Hue" "Color" "Saturation" "Value"))
         (modelist (list NORMAL-MODE ADDITION-MODE
                    SUBTRACT-MODE DIFFERENCE-MODE
                    MULTIPLY-MODE DIVIDE-MODE
                    DODGE-MODE BURN-MODE SCREEN-MODE OVERLAY-MODE
                    HARDLIGHT-MODE SOFTLIGHT-MODE
                    DARKEN-ONLY-MODE LIGHTEN-ONLY-MODE
                    GRAIN-EXTRACT-MODE GRAIN-MERGE-MODE
                    HUE-MODE COLOR-MODE SATURATION-MODE VALUE-MODE))

         ;; Make the new image
         (newimg (car (gimp-image-new pagew pageh RGB)))
         (bg (car (gimp-layer-new newimg pagew pageh
                                  RGBA-IMAGE "background"
                                  100 NORMAL-MODE)))
         (i 0)
        )
    (gimp-image-undo-disable newimg)
    (gimp-image-add-layer newimg bg -1)
    (gimp-context-set-foreground bgcolor)
    (gimp-drawable-fill bg FOREGROUND-FILL)
    (gimp-context-set-foreground textcolor)
    (gimp-image-set-filename newimg "allmodes")

    (while (< i nx)
           (set! j 0)
           (while (< j ny)
                  (gimp-edit-copy srcdrawable)
                  (let* ((pasted (car (gimp-edit-paste bg FALSE)))
                         (x (* i dx))
                         (y (* j dy))
                         (thismode (car modelist))
                         (thisname (car namelist))
                        )

                    ;; Make the pasted image into a new layer for this mode:
                    (gimp-layer-set-offsets pasted x y)
                    (gimp-floating-sel-to-layer pasted)
                    (gimp-layer-set-mode pasted thismode)
                    (gimp-drawable-set-name pasted thisname)

                    ;; Now make a text label corresponding to this layer.
                    (let* ((text-extents
                            (gimp-text-get-extents-fontname thisname
                                                            fontsize PIXELS
                                                            font))
                           (cx (+ x (/ imgw 2)))
                           (textx (- cx (/ (car text-extents) 2)))
                           ;(texty (- cy (/ (cadr text-extents) 2))))
                           (texty (+ y imgh))

                           (text-layer
                            (car (gimp-text-fontname
                                  newimg bg
                                  textx texty
                                  thisname
                                  5
                                  TRUE
                                  fontsize PIXELS font)))
                          )
                      (gimp-floating-sel-to-layer text-layer)
                    )
                  )
                  (set! modelist (cdr modelist))
                  (set! namelist (cdr namelist))
                  (set! j (+ j 1))
           )
           (set! i (+ i 1))
     )

    ;; Clean up
    (gimp-context-set-foreground old-fg-color)
    (gimp-image-undo-enable newimg)
    (gimp-display-new newimg)
  )
)

(script-fu-register "script-fu-all-layer-modes"
                     _"<Image>/Filters/Layer Mode Comparator..."
                    "Make a page showing an image in each layer mode against a constant background"
                    "Akkana Peck"
                    "Akkana Peck"
                    "August 2005"
                    ""
                    SF-IMAGE      "Image"          0
                    SF-DRAWABLE   "Drawable"       0
                    SF-COLOR      _"Background color"   '(128 128 128)
                    SF-FONT       _"Font"               "Arial Bold Italic"
                    SF-ADJUSTMENT _"Font size (pixels)" '(15 2 500 1 10 0 1)
                    SF-COLOR      _"Text color"         '(255 255 255)
)


