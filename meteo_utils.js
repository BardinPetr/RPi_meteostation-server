module.exports.c_to_f = (a) => Math.round(a * 9 / 5 + 32);
module.exports.f_to_c = (a) => Math.round((a - 32) * 5 / 9);

module.exports.mmHg_to_inHg = (a) => Math.round(a * 0.039481);
module.exports.inHg_to_mmHg = (a) => Math.round(a / 0.039481);
