using Plots
using Plots.PlotMeasures
using LambertW
import PlotlyBase

plotly()
Plots.PlotlyBackend()

Plots.scalefontsizes()
Plots.scalefontsizes(1.3)

R = 0:0.01:1;

## No-repair reliability
reliability_single = R;
reliability_dr = @. 1 - (1 - R)^2;
#reliability_3r = @. 1 - (1 - R)^3;
reliability_tmr = @. 3 * R^2 - 2 * R^3

plot(
    R,
    [reliability_single, reliability_dr, reliability_tmr],
    label = ["Απλό εξάρτημα" "Διπλός πλεονασμός" "Τριπλός πλεονασμός με ψηφοφορία"],
    # title = "Ανθεκτικότητα σύνθετου συστήματος σε μόνιμες βλάβες",
    legend = :outerright,
    xlabel = "Αξιοπιστία εξαρτήματος",
    ylabel = "Αξιοπιστία συστήματος",
    lw = 2,
)
savefig("reliability_norepair.pdf")

## No-repair reliability with vulnerable voter
Rv = 0:0.01:1;
reliability_tmrv = reliability_tmr * permutedims(Rv)

heatmap(
    R,
    Rv,
    reliability_tmrv,
    xlabel = "Αξιοπιστία R εξαρτήματος",
    ylabel = "Αξιοπιστία Rv ψηφοφόρου",
    colorbar_title = "Αξιοπιστία συστήματος",
    aspect_ratio = 1,
    c = :inferno
)
savefig("reliability_norepair_voter.pdf")

## Ρrepair reliability
λ = 10 .^ (LinRange(-11, -2, 1000))

λ_single = λ;
λ_dr = @. -lambertw(- (2λ * exp(-λ) - λ^2 * exp(-2λ)));
λ_tmr = @. -lambertw(- (3(λ * exp(-λ))^2 - 2(λ * exp(-λ))^3));

plot(
    λ,
    [λ_single, λ_single, λ_dr, λ_tmr],
    label = permutedims([
        "Απλό εξάρτημα"
        "Διπλός πλεονασμός, ψυχρός"
        "Διπλός πλεονασμός, ενεργός"
        "Τριπλός πλεονασμός με ψηφοφορία"]),
    #title = "Ανθεκτικότητα σύνθετου συστήματος σε παροδικά σφάλματα",
    legend = (0.6,0.35),
    xlabel = "Ρυθμός αποτυχίας εξαρτήματος",
    ylabel = "Ρυθμός αποτυχίας συστήματος",
    yaxis = :log,
    xaxis = :log,
    left_margin = -150px,
    lw = 2,
    linecolor=[palette(:tab10)[1] palette(:tab10)[1] palette(:tab10)[2] palette(:tab10)[3]]
)
savefig("reliability_repair.pdf")
