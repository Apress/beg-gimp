;; colorcircles.scm: Make three colored circles
;; to illustrate additive or subtractive color.
;;
;; Copyright (C) 2005 by Akkana Peck.
;; This program is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License.

(define (script-fu-color-circles diameter additive)
    (let* (
           ;; Calculate image size based on circle diameter
           (imagew (* 2 diameter))
           (imageh (* 2 diameter))
           (radius (/ diameter 2))
           ;; Offset of the top circle from the center
           (offset (* diameter .33))
           ;; Offsets of the lower two
           (yoffset (/ offset 2))
           (xoffset (* offset .866))
           ;; The layer mode we'll be using for the color circles
           (layermode (if (= additive TRUE) ADDITION-MODE SUBTRACT-MODE))
           ;; Make the new image
           (newimg (car (gimp-image-new imagew imageh RGB)))
           ;; and a background layer
           (bg (car (gimp-layer-new newimg imagew imageh
                                    RGBA-IMAGE "background"
                                    100 NORMAL-MODE)))
           ;; Save the old foreground color
           (old-fg-color (car (gimp-context-get-foreground)))
          )

      (gimp-image-undo-disable newimg)
      (gimp-image-add-layer newimg bg -1)
      (gimp-context-set-foreground (if (= additive TRUE)
                                       '(0 0 0)
                                       '(255 255 255)))
      (gimp-drawable-fill bg FOREGROUND-FILL)
      (if (= additive TRUE)
          (gimp-image-set-filename newimg "AdditiveColors")
          (gimp-image-set-filename newimg "SubtractiveColors")
      )

      ;; Draw the red circle
      (draw-circle newimg radius (- radius offset) diameter
                   '(255 0 0) "red" layermode)

      ;; Draw the green circle
      (draw-circle newimg (- radius xoffset) (+ radius yoffset) diameter
                   '(0 255 0) "green" layermode)

      ;; Draw the blue circle
      (draw-circle newimg (+ radius xoffset) (+ radius yoffset) diameter
                   '(0 0 255) "blue" layermode)

      ;; Clean up
      (gimp-context-set-foreground old-fg-color)
      (gimp-image-undo-enable newimg)
      (gimp-display-new newimg)
     )
)

(define (draw-circle img x y diameter color layername layermode)
  (let*
      ((newlayer (car (gimp-layer-new img
                                     (car (gimp-image-width img))
                                     (car (gimp-image-height img))
                                     RGBA-IMAGE layername
                                     100 NORMAL-MODE))))

      (gimp-image-add-layer img newlayer -1)
      (gimp-edit-clear newlayer)
      (gimp-ellipse-select img x y diameter diameter
                           CHANNEL-OP-REPLACE TRUE TRUE 0)
      (gimp-context-set-foreground color)
      (gimp-edit-fill newlayer FOREGROUND-FILL)
      (gimp-selection-none img)
      (gimp-layer-set-mode newlayer layermode)
  )
)

(script-fu-register "script-fu-color-circles"
                     _"<Toolbox>/Xtns/Misc/Color Circles Fu..."
                    "Make an image demonstrating additive color"
                    "Akkana Peck"
                    "Akkana Peck"
                    "August 2005"
                    ""
                    SF-ADJUSTMENT _"Diameter" '(200 50 500 1 10 0 1)
                    SF-TOGGLE     _"Additive" TRUE
)
