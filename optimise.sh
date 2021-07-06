gs  -q -dNOPAUSE -dBATCH -dSAFER \
    -sDEVICE=pdfwrite \
    -dCompatibilityLevel=1.5 \
    -dPDFSETTINGS=/printer \
    -dEmbedAllFonts=true \
    -dSubsetFonts=true \
    -sOutputFile=$1.compressed.pdf \
     $1

#    -dColorImageDownsampleType=/Bicubic \
#    -dColorImageResolution=72 \
#    -dGrayImageDownsampleType=/Bicubic \
#    -dGrayImageResolution=72 \
#    -dMonoImageDownsampleType=/Bicubic \
#    -dMonoImageResolution=72 \
