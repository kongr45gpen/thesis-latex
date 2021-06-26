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
    label = ["Απλό εξάρτημα" "Διπλός πλεονασμός" "Τριπλός πλεονασμός με ψηφοφορία"],
    title = "Ανθεκτικότητα σύνθετου συστήματος σε μόνιμες βλάβες",
    legend = :bottomright,
    xlabel = "Αξιοπιστία εξαρτήματος",
    ylabel = "Αξιοπιστία συστήματος",
    lw = 2
)
savefig("reliability_norepair.pdf")

##
λ = LinRange(0.0001, 0.01, 1000);

λ_single = λ;
λ_dr = @. -lambertw(-sqrt(exp(-λ)) * sqrt(λ));
λ_tmr = @. -lambertw(- exp(-2λ) * λ^2);

plot(
    λ,
    [λ_single, λ_single, λ_dr, λ_tmr],
    label = [
        "Απλό εξάρτημα"
        "Διπλός πλεονασμός, ψυχρός"
        "Διπλός πλεονασμός, ενεργός"
        "Τριπλός πλεονασμός με ψηφοφορία"],
    title = "Ανθεκτικότητα σύνθετου συστήματος σε παροδικά σφάλματα",
    legend = :bottomright,
    xlabel = "Ρυθμός αποτυχίας εξαρτήματος",
    ylabel = "Ρυθμός αποτυχίας συστήματος",
    yaxis = :log,
    lw = 2
)
savefig("reliability_repair.pdf")
