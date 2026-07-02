import 'package:flutter/material.dart';
import 'package:google_fonts/google_fonts.dart';
import '../theme.dart';
import '../widgets/status_bar.dart' show StatusBar;

enum _StreamState { streaming, idle }

const _txPowerOptions = [
  (dBm: -17, label: 'Minimum (test only)'),
  (dBm:   0, label: 'Low · 1 mW'),
  (dBm:  10, label: 'Medium · 10 mW'),
  (dBm:  14, label: 'High · 25 mW'),
  (dBm:  17, label: 'Very High · 50 mW'),
  (dBm:  22, label: 'Maximum · 158 mW'),
];

class RtcmStreamingScreen extends StatefulWidget {
  const RtcmStreamingScreen({super.key});

  @override
  State<RtcmStreamingScreen> createState() => _RtcmStreamingScreenState();
}

class _RtcmStreamingScreenState extends State<RtcmStreamingScreen> {
  // Mock state — streaming
  final _StreamState _state = _StreamState.streaming;

  // GFSK config (would be read from BLE GFSK_RADIO_CONFIG 0xAD0A on open)
  double _freqMhz = 434.000;
  int    _txPowerIdx = 1;   // 0 dBm selected
  int    _bitRateBps = 9600;
  int    _syncWord  = 0x906F26;

  // Derived (read-only, computed from bit rate)
  int get _freqDevHz => _bitRateBps ~/ 2;
  int get _rxBwHz {
    final min = 2 * _freqDevHz + _bitRateBps;
    // Next SX1262 RX BW step above min (simplified)
    for (final step in [4800, 5800, 7300, 9700, 11700, 14600, 19500,
                        23400, 29300, 39000, 46900, 58600, 78200,
                        93800, 117300, 156200, 187200, 234300, 312000,
                        373600, 467000]) {
      if (step > min) return step;
    }
    return 467000;
  }

  String get _syncWordStr => '0x${_syncWord.toRadixString(16).toUpperCase()}';

  void _editFreq() async {
    final result = await _showNumericDialog(
      context: context,
      title: 'Frequency (MHz)',
      initial: _freqMhz.toStringAsFixed(3),
    );
    if (result != null) {
      final v = double.tryParse(result);
      if (v != null) setState(() => _freqMhz = v);
    }
  }

  void _editBitRate() async {
    final result = await _showNumericDialog(
      context: context,
      title: 'Bit Rate (bps)',
      initial: _bitRateBps.toString(),
    );
    if (result != null) {
      final v = int.tryParse(result);
      if (v != null && v > 0) setState(() => _bitRateBps = v);
    }
  }

  void _editSyncWord() async {
    final result = await _showNumericDialog(
      context: context,
      title: 'Sync Word (hex, max 4 bytes)',
      initial: _syncWord.toRadixString(16).toUpperCase(),
      hex: true,
    );
    if (result != null) {
      final v = int.tryParse(result.replaceAll('0x', '').replaceAll('0X', ''), radix: 16);
      if (v != null) setState(() => _syncWord = v);
    }
  }

  @override
  Widget build(BuildContext context) {
    final isStreaming = _state == _StreamState.streaming;
    final badgeBg = isStreaming
        ? const Color(0xFFE8F5E9)
        : const Color(0xFFF5F5F5);
    final badgeFg = isStreaming
        ? const Color(0xFF2E7D32)
        : const Color(0xFF888888);
    final badgeText = isStreaming ? '● Streaming' : '○ Idle';

    return Scaffold(
      backgroundColor: kLightBg,
      resizeToAvoidBottomInset: false,
      body: Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          const StatusBar(),
          // Header with state badge
          Container(
            height: 52,
            color: kLightHeaderBg,
            padding: const EdgeInsets.symmetric(horizontal: 16),
            child: Row(
              children: [
                GestureDetector(
                  onTap: () => Navigator.pop(context),
                  behavior: HitTestBehavior.opaque,
                  child: Padding(
                    padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 8),
                    child: Text('←',
                        style: GoogleFonts.montserrat(
                            fontSize: 24, color: Colors.white, height: 1)),
                  ),
                ),
                const SizedBox(width: 16),
                Expanded(
                  child: Text('RTCM Data',
                      style: GoogleFonts.montserrat(
                          fontSize: 20, fontWeight: FontWeight.w700,
                          color: Colors.white)),
                ),
                Container(
                  padding: const EdgeInsets.symmetric(
                      horizontal: 12, vertical: 3),
                  decoration: BoxDecoration(
                    color: badgeBg,
                    borderRadius: BorderRadius.circular(12),
                  ),
                  child: Text(badgeText,
                      style: GoogleFonts.montserrat(
                          fontSize: 13, fontWeight: FontWeight.w700,
                          color: badgeFg)),
                ),
              ],
            ),
          ),
          Expanded(
            child: Row(
              crossAxisAlignment: CrossAxisAlignment.stretch,
              children: [
                _StatsPanel(streaming: isStreaming),
                Expanded(
                  child: _GfskPanel(
                    freqMhz: _freqMhz,
                    txPowerIdx: _txPowerIdx,
                    bitRateBps: _bitRateBps,
                    syncWordStr: _syncWordStr,
                    freqDevHz: _freqDevHz,
                    rxBwHz: _rxBwHz,
                    onEditFreq: _editFreq,
                    onTxPowerChanged: (i) => setState(() => _txPowerIdx = i),
                    onEditBitRate: _editBitRate,
                    onEditSyncWord: _editSyncWord,
                    onStop: () => Navigator.pop(context),
                  ),
                ),
              ],
            ),
          ),
        ],
      ),
    );
  }
}

// ── Left panel — broadcast statistics ────────────────────────────────────────

class _StatsPanel extends StatelessWidget {
  const _StatsPanel({required this.streaming});
  final bool streaming;

  @override
  Widget build(BuildContext context) {
    return Container(
      width: 430,
      decoration: const BoxDecoration(
        border: Border(right: BorderSide(color: Color(0xFFE0E0E0))),
      ),
      padding: const EdgeInsets.all(16),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          _SectionLabel('Broadcast rate'),
          const SizedBox(height: 8),
          Row(
            children: [
              Expanded(child: _StatBox(
                  value: streaming ? '47' : '0',
                  unit: 'packets / sec',
                  active: streaming)),
              const SizedBox(width: 10),
              Expanded(child: _StatBox(
                  value: streaming ? '4.2' : '0.0',
                  unit: 'KB / sec',
                  active: streaming)),
            ],
          ),
          const SizedBox(height: 14),
          _SectionLabel('RTCM messages active'),
          const SizedBox(height: 8),
          Row(
            children: [
              Expanded(child: _MsgChip(
                  id: '1005', desc: 'Base ARP',
                  rate: '0.2 Hz', active: streaming)),
              const SizedBox(width: 10),
              Expanded(child: _MsgChip(
                  id: '1074', desc: 'GPS MSM4',
                  rate: '1 Hz', active: streaming)),
              const SizedBox(width: 10),
              Expanded(child: _MsgChip(
                  id: '1084', desc: 'GLO MSM4',
                  rate: '1 Hz', active: streaming)),
            ],
          ),
          const SizedBox(height: 14),
          _SectionLabel('Session totals'),
          const SizedBox(height: 8),
          _TotalsGrid(streaming: streaming),
        ],
      ),
    );
  }
}

class _StatBox extends StatelessWidget {
  const _StatBox({required this.value, required this.unit, required this.active});
  final String value;
  final String unit;
  final bool active;

  @override
  Widget build(BuildContext context) {
    return Container(
      padding: const EdgeInsets.symmetric(horizontal: 14, vertical: 10),
      decoration: BoxDecoration(
        color: const Color(0xFFF5F5F5),
        borderRadius: BorderRadius.circular(8),
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text(value,
              style: GoogleFonts.montserrat(
                  fontSize: 28, fontWeight: FontWeight.w700,
                  color: active
                      ? const Color(0xFF1B5E20)
                      : const Color(0xFF9E9E9E),
                  height: 1.1)),
          const SizedBox(height: 2),
          Text(unit,
              style: GoogleFonts.montserrat(
                  fontSize: 12, color: const Color(0xFF757575))),
        ],
      ),
    );
  }
}

class _MsgChip extends StatelessWidget {
  const _MsgChip({
    required this.id,
    required this.desc,
    required this.rate,
    required this.active,
  });
  final String id;
  final String desc;
  final String rate;
  final bool active;

  @override
  Widget build(BuildContext context) {
    final borderColor = active
        ? const Color(0xFF43A047)
        : const Color(0xFFBDBDBD);
    final bg = active
        ? const Color(0xFFE8F5E9)
        : const Color(0xFFF5F5F5);
    final idColor = active
        ? const Color(0xFF1B5E20)
        : const Color(0xFF9E9E9E);
    final rateColor = active
        ? const Color(0xFF43A047)
        : const Color(0xFFBDBDBD);

    return Container(
      padding: const EdgeInsets.all(10),
      decoration: BoxDecoration(
        color: bg,
        borderRadius: BorderRadius.circular(8),
        border: Border.all(color: borderColor, width: 2),
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text(id,
              style: GoogleFonts.montserrat(
                  fontSize: 20, fontWeight: FontWeight.w700,
                  color: idColor)),
          Text(desc,
              style: GoogleFonts.montserrat(
                  fontSize: 11, color: const Color(0xFF757575))),
          const SizedBox(height: 4),
          Text(rate,
              style: GoogleFonts.montserrat(
                  fontSize: 11, fontWeight: FontWeight.w700,
                  color: rateColor)),
        ],
      ),
    );
  }
}

class _TotalsGrid extends StatelessWidget {
  const _TotalsGrid({required this.streaming});
  final bool streaming;

  @override
  Widget build(BuildContext context) {
    return Row(
      children: [
        Expanded(child: _TotalBox(
            value: streaming ? '12,847' : '0',
            label: 'Packets sent')),
        const SizedBox(width: 8),
        Expanded(child: _TotalBox(
            value: streaming ? '1.08 MB' : '0 B',
            label: 'Bytes forwarded')),
      ],
    );
  }
}

class _TotalBox extends StatelessWidget {
  const _TotalBox({required this.value, required this.label});
  final String value;
  final String label;

  @override
  Widget build(BuildContext context) {
    return Container(
      padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 8),
      decoration: BoxDecoration(
        color: const Color(0xFFF5F5F5),
        borderRadius: BorderRadius.circular(6),
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text(value,
              style: GoogleFonts.montserrat(
                  fontSize: 18, fontWeight: FontWeight.w700,
                  color: const Color(0xFF37474F))),
          const SizedBox(height: 1),
          Text(label,
              style: GoogleFonts.montserrat(
                  fontSize: 11, color: const Color(0xFF9E9E9E))),
        ],
      ),
    );
  }
}

// ── Right panel — GFSK configuration ─────────────────────────────────────────

class _GfskPanel extends StatelessWidget {
  const _GfskPanel({
    required this.freqMhz,
    required this.txPowerIdx,
    required this.bitRateBps,
    required this.syncWordStr,
    required this.freqDevHz,
    required this.rxBwHz,
    required this.onEditFreq,
    required this.onTxPowerChanged,
    required this.onEditBitRate,
    required this.onEditSyncWord,
    required this.onStop,
  });

  final double freqMhz;
  final int txPowerIdx;
  final int bitRateBps;
  final String syncWordStr;
  final int freqDevHz;
  final int rxBwHz;
  final VoidCallback onEditFreq;
  final ValueChanged<int> onTxPowerChanged;
  final VoidCallback onEditBitRate;
  final VoidCallback onEditSyncWord;
  final VoidCallback onStop;

  @override
  Widget build(BuildContext context) {
    final txOpt = _txPowerOptions[txPowerIdx];

    return Padding(
      padding: const EdgeInsets.all(16),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          _SectionLabel('GFSK Radio'),
          const SizedBox(height: 8),

          // Frequency
          _EditableField(
            name: 'Frequency',
            value: '${freqMhz.toStringAsFixed(3)} MHz',
            onTap: onEditFreq,
          ),
          const SizedBox(height: 10),

          // TX Power — dropdown via PopupMenuButton
          _TxPowerField(
            selected: txPowerIdx,
            onChanged: onTxPowerChanged,
          ),
          const SizedBox(height: 10),

          // Bit Rate
          _EditableField(
            name: 'Bit Rate',
            value: '$bitRateBps bps',
            onTap: onEditBitRate,
          ),
          const SizedBox(height: 10),

          // Sync Word
          _EditableField(
            name: 'Sync Word',
            value: syncWordStr,
            mono: true,
            onTap: onEditSyncWord,
          ),
          const SizedBox(height: 10),

          // Read-only derived fields
          Row(
            children: [
              Expanded(child: _DerivedField(
                  name: 'Freq Deviation',
                  value: '$freqDevHz Hz')),
              const SizedBox(width: 8),
              Expanded(child: _DerivedField(
                  name: 'RX Bandwidth',
                  value: '$rxBwHz Hz')),
            ],
          ),

          const Spacer(),

          // Stop streaming button
          SizedBox(
            height: 52,
            child: ElevatedButton(
              onPressed: onStop,
              style: ElevatedButton.styleFrom(
                backgroundColor: const Color(0xFFB71C1C),
                foregroundColor: Colors.white,
                elevation: 0,
                shape: RoundedRectangleBorder(
                    borderRadius: BorderRadius.circular(8)),
              ),
              child: Text('Stop Streaming',
                  style: GoogleFonts.montserrat(
                      fontSize: 16, fontWeight: FontWeight.w700)),
            ),
          ),
        ],
      ),
    );
  }
}

class _EditableField extends StatelessWidget {
  const _EditableField({
    required this.name,
    required this.value,
    required this.onTap,
    this.mono = false,
  });

  final String name;
  final String value;
  final VoidCallback onTap;
  final bool mono;

  @override
  Widget build(BuildContext context) {
    return Material(
      color: const Color(0xFFF5F5F5),
      borderRadius: BorderRadius.circular(8),
      child: InkWell(
        onTap: onTap,
        borderRadius: BorderRadius.circular(8),
        child: Container(
          padding: const EdgeInsets.symmetric(horizontal: 14, vertical: 10),
          decoration: BoxDecoration(
            borderRadius: BorderRadius.circular(8),
            border: Border.all(color: const Color(0xFFE0E0E0)),
          ),
          child: Row(
            children: [
              Expanded(
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Text(name,
                        style: GoogleFonts.montserrat(
                            fontSize: 11, color: const Color(0xFF9E9E9E))),
                    const SizedBox(height: 2),
                    Text(value,
                        style: mono
                            ? const TextStyle(
                                fontSize: 20,
                                fontWeight: FontWeight.w700,
                                color: Color(0xFF212121),
                                fontFamily: 'monospace',
                                letterSpacing: 0.05,
                              )
                            : GoogleFonts.montserrat(
                                fontSize: 20, fontWeight: FontWeight.w700,
                                color: const Color(0xFF212121))),
                  ],
                ),
              ),
              const Icon(Icons.edit, size: 18, color: Color(0xFFBDBDBD)),
            ],
          ),
        ),
      ),
    );
  }
}

class _TxPowerField extends StatelessWidget {
  const _TxPowerField({required this.selected, required this.onChanged});
  final int selected;
  final ValueChanged<int> onChanged;

  @override
  Widget build(BuildContext context) {
    final opt = _txPowerOptions[selected];
    final displayVal = opt.dBm >= 0 ? '+${opt.dBm}' : '${opt.dBm}';

    return PopupMenuButton<int>(
      initialValue: selected,
      onSelected: onChanged,
      offset: const Offset(0, 52),
      shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(8)),
      itemBuilder: (_) => [
        for (int i = 0; i < _txPowerOptions.length; i++)
          PopupMenuItem<int>(
            value: i,
            child: Row(
              children: [
                SizedBox(
                  width: 80,
                  child: Text(
                    '${_txPowerOptions[i].dBm >= 0 ? '+' : ''}${_txPowerOptions[i].dBm} dBm',
                    style: GoogleFonts.montserrat(
                        fontSize: 18, fontWeight: FontWeight.w700,
                        color: i == selected
                            ? const Color(0xFF2E7D32)
                            : const Color(0xFF212121)),
                  ),
                ),
                const SizedBox(width: 12),
                Text(_txPowerOptions[i].label,
                    style: GoogleFonts.montserrat(
                        fontSize: 13,
                        color: i == selected
                            ? const Color(0xFF43A047)
                            : const Color(0xFF9E9E9E))),
              ],
            ),
          ),
      ],
      child: Container(
        padding: const EdgeInsets.symmetric(horizontal: 14, vertical: 10),
        decoration: BoxDecoration(
          color: const Color(0xFFF5F5F5),
          borderRadius: BorderRadius.circular(8),
          border: Border.all(color: const Color(0xFFE0E0E0)),
        ),
        child: Row(
          children: [
            Expanded(
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  Text('TX Power',
                      style: GoogleFonts.montserrat(
                          fontSize: 11, color: const Color(0xFF9E9E9E))),
                  const SizedBox(height: 2),
                  Text('$displayVal dBm  ·  ${opt.label}',
                      style: GoogleFonts.montserrat(
                          fontSize: 20, fontWeight: FontWeight.w700,
                          color: const Color(0xFF212121))),
                ],
              ),
            ),
            const Icon(Icons.arrow_drop_down,
                size: 24, color: Color(0xFFBDBDBD)),
          ],
        ),
      ),
    );
  }
}

class _DerivedField extends StatelessWidget {
  const _DerivedField({required this.name, required this.value});
  final String name;
  final String value;

  @override
  Widget build(BuildContext context) {
    return Container(
      padding: const EdgeInsets.symmetric(horizontal: 14, vertical: 8),
      decoration: BoxDecoration(
        color: const Color(0xFFFAFAFA),
        borderRadius: BorderRadius.circular(8),
        border: Border.all(color: const Color(0xFFEEEEEE)),
      ),
      child: Row(
        children: [
          Expanded(
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Text(name,
                    style: GoogleFonts.montserrat(
                        fontSize: 11, color: const Color(0xFFBDBDBD))),
                const SizedBox(height: 2),
                Text(value,
                    style: GoogleFonts.montserrat(
                        fontSize: 16, fontWeight: FontWeight.w700,
                        color: const Color(0xFF9E9E9E))),
              ],
            ),
          ),
          Container(
            padding: const EdgeInsets.symmetric(horizontal: 6, vertical: 2),
            decoration: BoxDecoration(
              border: Border.all(color: const Color(0xFFE0E0E0)),
              borderRadius: BorderRadius.circular(4),
            ),
            child: Text('AUTO',
                style: GoogleFonts.montserrat(
                    fontSize: 10, fontWeight: FontWeight.w700,
                    color: const Color(0xFFBDBDBD))),
          ),
        ],
      ),
    );
  }
}

// ── Shared helpers ────────────────────────────────────────────────────────────

class _SectionLabel extends StatelessWidget {
  const _SectionLabel(this.text);
  final String text;

  @override
  Widget build(BuildContext context) {
    return Text(text.toUpperCase(),
        style: GoogleFonts.montserrat(
            fontSize: 11, fontWeight: FontWeight.w700,
            letterSpacing: 1, color: const Color(0xFF999999)));
  }
}

// ── Simple numeric/hex edit dialog ───────────────────────────────────────────

Future<String?> _showNumericDialog({
  required BuildContext context,
  required String title,
  required String initial,
  bool hex = false,
}) {
  final ctrl = TextEditingController(text: initial);
  return showDialog<String>(
    context: context,
    builder: (_) => AlertDialog(
      title: Text(title,
          style: GoogleFonts.montserrat(
              fontSize: 18, fontWeight: FontWeight.w700)),
      content: TextField(
        controller: ctrl,
        autofocus: true,
        keyboardType: hex
            ? TextInputType.text
            : const TextInputType.numberWithOptions(
                decimal: true, signed: true),
        style: GoogleFonts.montserrat(fontSize: 20, color: kLightText),
        decoration: InputDecoration(
          prefix: hex
              ? Text('0x',
                  style: GoogleFonts.montserrat(
                      fontSize: 20, color: const Color(0xFF9E9E9E)))
              : null,
          border: const OutlineInputBorder(),
        ),
      ),
      actions: [
        TextButton(
          onPressed: () => Navigator.pop(context),
          child: Text('Cancel',
              style: GoogleFonts.montserrat(color: const Color(0xFF757575))),
        ),
        ElevatedButton(
          onPressed: () => Navigator.pop(context, ctrl.text),
          style: ElevatedButton.styleFrom(
              backgroundColor: kBtnGreen, foregroundColor: Colors.white),
          child: Text('Confirm',
              style: GoogleFonts.montserrat(fontWeight: FontWeight.w700)),
        ),
      ],
    ),
  );
}
