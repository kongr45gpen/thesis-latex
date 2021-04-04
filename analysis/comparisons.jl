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
    title = "MTTR = ∞",
    legend = :bottomright,
    xlabel = "Reliability",
)

λ = LinRange(0.0001, 0.01, 1000);

λ_single = λ;
λ_dr = @. max(-lambertw(exp(-2λ) * λ^2), 0.000000001)
λ_tmr = λ;

plot(
    R,
    [reliability_single, reliability_dr, reliability_tmr],
    label = ["Single component" "2 redundancy" "3ple modular redundancy"],
    title = "MTTR = ∞",
    legend = :bottomright,
    xlabel = "Reliability",
)
