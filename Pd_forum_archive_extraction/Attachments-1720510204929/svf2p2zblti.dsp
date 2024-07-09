import("stdfaust.lib");
// 2p2z state-variable filter with zero-delay feedback topology
// inputs are cutoff, Q, shelving boost (linear amp.), input signal
// outputs are LP, HP, BP, BP_normalised, LS, HS, BS, notch, peak AP
svf2blti(cf, q, k, in) =   tick
                           ~ ( _,
                               _) : (  !,
                                       !,
                                       _,
                                       _,
                                       _,
                                       _,
                                       _,
                                       _,
                                       _,
                                       _,
                                       _,
                                       _)
with {
    w(f) = 2*ma.PI*f/ma.SR;
    r = 1/2*(1/q);
    g = w(cf)/2;
    tick(s1, s2) =  u1,
                    u2,
                    lp,
                    hp,
                    bp,
                    bp_norm,
                    ls,
                    hs,
                    b_shelf,
                    notch,
                    peak,
                    ap
    with {
        u1 = v1+bp;
        u2 = v2+lp;
        v1 = hp*g;
        v2 = bp*g;
        hp = (in-s1*(2*r+g)-s2)/(1+2*r+g*g);
        bp = s1+v1;
        lp = s2+v2;
        bp_norm = bp*2*r;
        b_shelf = in+k*bp_norm;
        ls = in+k*lp;
        hs = in+k*hp;
        notch = in-bp_norm;
        ap = in-4*r*bp;
        peak = lp-hp;
    };
};
process = svf2blti;