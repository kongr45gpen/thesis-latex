using Plots
using LambertW

R = 0:0.01:1;

reliability_single = R;
reliability_dr = @. 1 - (1 - R)^2;
#reliability_tmr = @. 1 - (1 - R)^3;
reliability_tmr = @. 1 - (3 * (1 - R)^2 - 2 * (1 - R)^3);

plot(
    R,
    [reliability_single, reliability_dr, reliability_tmr],
    label = ["Single component" "2 redundancy" "3ple modular redundancy"],
    title = "MTTR = infty",
    legend = :bottomright,
    xlabel = "Reliability",
    lw = 2
)

##
λ = LinRange(0.0001, 0.01, 1000);

λ_single = λ;
λ_dr = @. -lambertw(-sqrt(exp(-λ)) * sqrt(λ));
λ_tmr = @. -lambertw(- exp(-2λ) * λ^2);

plot(
    λ,
    [λ_single, λ_single, λ_dr, λ_tmr],
    label = ["Single component" "2 redundancy, cold" "2 redundancy, hot" "3ple modular redundancy"],
    title = "MTTR = 0",
    legend = :bottomright,
    xlabel = "Failure rate",
    yaxis = :log,
    lw = 2
)
