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
reliability_3r = @. 1 - (1 - R)^3;
reliability_tmr = @. 3 * R^2 - 2 * R^3

plot(
    R,
    [reliability_single, reliability_dr, reliability_tmr, reliability_3r],
    label = ["Single component" "Dual redundancy" "Triple modular redundancy (naive)" "Triple modular redundancy (resilient)"],
    # title = "Resiliency to permanent failures",
    legend = :outerright,
    xlabel = "Component reliability",
    ylabel = "System reliability",
    lw = 2,
)
savefig("reliability_norepair_en.pdf")

## No-repair reliability with vulnerable voter
Rv = 0:0.01:1;
reliability_tmrv = reliability_tmr * permutedims(Rv)

heatmap(
    R,
    Rv,
    reliability_tmrv,
    xlabel = "Component reliability R",
    ylabel = "Voter reliability Rv",
    colorbar_title = "System reliability",
    aspect_ratio = 1,
    c = :inferno
)
savefig("reliability_norepair_voter_en.pdf")

## Ρrepair reliability
λ = 10 .^ (LinRange(-11, -2, 1000))

λ_single = λ;
λ_dr = @. -lambertw(- (2λ * exp(-λ) - λ^2 * exp(-2λ)));
λ_tmr = @. -lambertw(- (3(λ * exp(-λ))^2 - 2(λ * exp(-λ))^3));

plot(
    λ,
    [λ_single, λ_single, λ_dr, λ_tmr],
    label = permutedims([
        "Single component"
        "Dual redundancy, cold"
        "Dual redundancy, hot"
        "Triple modular redundancy"]),
    #title = "Resilience to temporary failures",
    legend = (0.6,0.35),
    xlabel = "Component failure rate",
    ylabel = "System failure rate",
    yaxis = :log,
    xaxis = :log,
    left_margin = -150px,
    lw = 2,
    linecolor=[palette(:tab10)[1] palette(:tab10)[1] palette(:tab10)[2] palette(:tab10)[3]]
)
savefig("reliability_repair_en.pdf")
