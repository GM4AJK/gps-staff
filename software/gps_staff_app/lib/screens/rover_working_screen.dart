import 'package:flutter/material.dart';
import 'package:google_fonts/google_fonts.dart';
import '../theme.dart';
import '../widgets/status_bar.dart';

enum _FixState { noFix, rtkFloat, rtkFix }

class RoverWorkingScreen extends StatefulWidget {
	const RoverWorkingScreen({
		super.key,
		required this.sessionName,
		required this.sessionTitle,
	});

	final String sessionName;
	final String sessionTitle;

	@override
	State<RoverWorkingScreen> createState() => _RoverWorkingScreenState();
}

class _RoverWorkingScreenState extends State<RoverWorkingScreen> {
	_FixState _fixState = _FixState.rtkFix;
	final List<_CapturedPoint> _points = const [
		_CapturedPoint(id: 'P001', label: 'P001', lat: '57.1234562°', lon: '−2.3456784°', height: '143.418 m', hacc: '9', vacc: '13'),
		_CapturedPoint(id: 'P002', label: 'BM-A', lat: '57.1234455°', lon: '−2.3456901°', height: '143.409 m', hacc: '7', vacc: '11', labelHighlight: true),
		_CapturedPoint(id: 'P003', label: 'P003', lat: '57.1234678°', lon: '−2.3456712°', height: '143.424 m', hacc: '8', vacc: '12'),
	];

	({Color bg, Color border, Color label, Color textColor, String text,
		String hacc, Color haccColor}) get _fixData => switch (_fixState) {
		_FixState.noFix => (
			bg: const Color(0xFF1A0A0A),
			border: const Color(0xFF4E1010),
			label: const Color(0xFF7F2020),
			textColor: const Color(0xFFEF9A9A),
			text: 'No Fix',
			hacc: '— mm',
			haccColor: const Color(0xFFEF9A9A),
		),
		_FixState.rtkFloat => (
			bg: const Color(0xFF1A0F00),
			border: const Color(0xFF5E3500),
			label: const Color(0xFF7E4A00),
			textColor: const Color(0xFFFFCC80),
			text: 'RTK Float',
			hacc: '142 mm',
			haccColor: const Color(0xFFFFCC80),
		),
		_FixState.rtkFix => (
			bg: const Color(0xFF1B5E20),
			border: const Color(0xFF2E7D32),
			label: const Color(0xFF388E3C),
			textColor: const Color(0xFFA5D6A7),
			text: 'RTK Fix',
			hacc: '8 mm',
			haccColor: const Color(0xFFA5D6A7),
		),
	};

	({String lat, String lon, String height, String hacc, String vacc, String sats}) get _posData =>
		switch (_fixState) {
			_FixState.noFix => (lat: '—', lon: '—', height: '—', hacc: '— mm', vacc: '— mm', sats: '—'),
			_FixState.rtkFloat => (lat: '57.1234498°', lon: '−2.3456721°', height: '143.318 m', hacc: '142 mm', vacc: '190 mm', sats: '16'),
			_FixState.rtkFix => (lat: '57.1234567°', lon: '−2.3456789°', height: '143.421 m', hacc: '8 mm', vacc: '12 mm', sats: '18'),
		};

	Future<void> _confirmEndSession(BuildContext context) async {
		final confirmed = await showDialog<bool>(
			context: context,
			builder: (ctx) => AlertDialog(
				backgroundColor: const Color(0xFF1A2030),
				shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(12)),
				title: Text('End Session?',
						style: GoogleFonts.montserrat(
								fontSize: 18,
								fontWeight: FontWeight.w700,
								color: const Color(0xFFECEFF1))),
				content: Text(
					'This will close the session and return to the home screen. '
					'Captured points have already been written to the SD card.',
					style: GoogleFonts.montserrat(
							fontSize: 13,
							color: const Color(0xFF78909C),
							height: 1.6),
				),
				actions: [
					ElevatedButton(
						onPressed: () => Navigator.pop(ctx, false),
						style: ElevatedButton.styleFrom(
							backgroundColor: const Color(0xFF2E7D32),
							foregroundColor: Colors.white,
							elevation: 0,
							shape: RoundedRectangleBorder(
									borderRadius: BorderRadius.circular(6)),
						),
						child: Text('Keep Going',
								style: GoogleFonts.montserrat(
										fontWeight: FontWeight.w700)),
					),
					const SizedBox(width: 16),
					ElevatedButton(
						onPressed: () => Navigator.pop(ctx, true),
						style: ElevatedButton.styleFrom(
							backgroundColor: const Color(0xFFC62828),
							foregroundColor: Colors.white,
							elevation: 0,
							shape: RoundedRectangleBorder(
									borderRadius: BorderRadius.circular(6)),
						),
						child: Text('End Session',
								style: GoogleFonts.montserrat(
										fontWeight: FontWeight.w700)),
					),
				],
			),
		);
		if (confirmed == true && context.mounted) Navigator.pop(context);
	}

	@override
	Widget build(BuildContext context) {
		final fix = _fixData;
		final pos = _posData;
		final canCapture = _fixState == _FixState.rtkFix;

		return Scaffold(
			backgroundColor: kBgHome,
      resizeToAvoidBottomInset: false,
			body: Column(
				crossAxisAlignment: CrossAxisAlignment.stretch,
				children: [
					_DualStatusBar(
						baseBatt: '91%',
						roverBatt: '72%',
						tabletBatt: '78%',
					),
					_SessionHeader(
						sessionName: widget.sessionName,
						elapsed: '00:12:34',
						pointCount: _points.length,
						onBack: () => Navigator.pop(context),
					),
					Expanded(
						child: Row(
							crossAxisAlignment: CrossAxisAlignment.stretch,
							children: [
								_LeftPanel(
									sessionTitle: widget.sessionTitle,
									pos: pos,
									fixState: _fixState,
									onEndSession: () => _confirmEndSession(context),
								),
								Expanded(
									child: _RightPanel(
										fix: fix,
										fixState: _fixState,
										onFixChange: (s) =>
												setState(() => _fixState = s),
										points: _points,
										canCapture: canCapture,
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

// ── Dual status bar ───────────────────────────────────────────────────────────

class _DualStatusBar extends StatelessWidget {
	const _DualStatusBar({
		required this.baseBatt,
		required this.roverBatt,
		required this.tabletBatt,
	});
	final String baseBatt;
	final String roverBatt;
	final String tabletBatt;

	@override
	Widget build(BuildContext context) {
		return Container(
			height: 28,
			color: kBgStatus,
			padding: const EdgeInsets.symmetric(horizontal: 10),
			child: Row(
				mainAxisAlignment: MainAxisAlignment.spaceBetween,
				children: [
					Row(
						children: [
							Text('Base ',
									style: GoogleFonts.montserrat(
											fontSize: 11, color: kTextMuted)),
							Text('▐████▌ $baseBatt',
									style: const TextStyle(
											fontFamily: 'monospace',
											fontSize: 11,
											color: Color(0xFF66BB6A))),
							const SizedBox(width: 16),
							Text('Rover ',
									style: GoogleFonts.montserrat(
											fontSize: 11, color: kTextMuted)),
							Text('▐███░▌ $roverBatt',
									style: const TextStyle(
											fontFamily: 'monospace',
											fontSize: 11,
											color: Color(0xFF66BB6A))),
						],
					),
					Text('▐███░▌ $tabletBatt',
							style: const TextStyle(
									fontFamily: 'monospace',
									fontSize: 11,
									color: Color(0xFF66BB6A))),
				],
			),
		);
	}
}

// ── Session header ────────────────────────────────────────────────────────────

class _SessionHeader extends StatelessWidget {
	const _SessionHeader({
		required this.sessionName,
		required this.elapsed,
		required this.pointCount,
		required this.onBack,
	});

	final String sessionName;
	final String elapsed;
	final int pointCount;
	final VoidCallback onBack;

	@override
	Widget build(BuildContext context) {
		return Container(
			height: 44,
			padding: const EdgeInsets.symmetric(horizontal: 14),
			decoration: const BoxDecoration(
				color: kBgStatus,
				border: Border(
						bottom: BorderSide(color: Color(0xFF1E2D3D))),
			),
			child: Row(
				children: [
					GestureDetector(
						onTap: onBack,
						behavior: HitTestBehavior.opaque,
						child: Padding(
							padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 8),
							child: Text('←',
									style: GoogleFonts.montserrat(
											fontSize: 20, color: kTextDim, height: 1)),
						),
					),
					const SizedBox(width: 12),
					Expanded(
						child: Text(sessionName,
								style: GoogleFonts.montserrat(
										fontSize: 14,
										fontWeight: FontWeight.w700,
										color: const Color(0xFF78909C))),
					),
					Text(elapsed,
							style: const TextStyle(
									fontFamily: 'monospace',
									fontSize: 14,
									color: Color(0xFF37474F))),
					const SizedBox(width: 12),
					Container(
						padding: const EdgeInsets.symmetric(
								horizontal: 10, vertical: 3),
						decoration: BoxDecoration(
							color: const Color(0xFF1A2640),
							border: Border.all(color: const Color(0xFF1E2D3D)),
							borderRadius: BorderRadius.circular(4),
						),
						child: Text(
							'$pointCount pts',
							style: GoogleFonts.montserrat(
									fontSize: 12,
									fontWeight: FontWeight.w700,
									color: kTextMuted),
						),
					),
				],
			),
		);
	}
}

// ── Left panel — position ─────────────────────────────────────────────────────

class _LeftPanel extends StatelessWidget {
	const _LeftPanel({
		required this.sessionTitle,
		required this.pos,
		required this.fixState,
		required this.onEndSession,
	});

	final String sessionTitle;
	final ({String lat, String lon, String height, String hacc, String vacc, String sats}) pos;
	final _FixState fixState;
	final VoidCallback onEndSession;

	Color get _accColor => switch (fixState) {
		_FixState.rtkFix => const Color(0xFF66BB6A),
		_FixState.rtkFloat => const Color(0xFFFFA726),
		_FixState.noFix => const Color(0xFF78909C),
	};

	@override
	Widget build(BuildContext context) {
		return Container(
			width: 420,
			padding: const EdgeInsets.fromLTRB(20, 20, 20, 16),
			decoration: const BoxDecoration(
				border: Border(right: BorderSide(color: Color(0xFF1E2D3D))),
			),
			child: Column(
				crossAxisAlignment: CrossAxisAlignment.start,
				children: [
					SizedBox(
						width: double.infinity,
						height: 52,
						child: ElevatedButton(
							onPressed: onEndSession,
							style: ElevatedButton.styleFrom(
								backgroundColor: const Color(0xFFC62828),
								foregroundColor: Colors.white,
								elevation: 0,
								shape: RoundedRectangleBorder(
										borderRadius: BorderRadius.circular(8)),
							),
							child: Text('End Session',
									style: GoogleFonts.montserrat(
											fontSize: 15, fontWeight: FontWeight.w700)),
						),
					),
					Expanded(
						child: Column(
							crossAxisAlignment: CrossAxisAlignment.start,
							mainAxisAlignment: MainAxisAlignment.center,
							children: [
								_CoordBlock(label: 'Session', value: sessionTitle, large: false),
								const SizedBox(height: 18),
								_CoordBlock(label: 'Pole height', value: '1.80 m', large: false),
								const SizedBox(height: 18),
								_CoordBlock(label: 'Latitude', value: pos.lat),
								const SizedBox(height: 14),
								_CoordBlock(label: 'Longitude', value: pos.lon),
								const SizedBox(height: 14),
								_CoordBlock(label: 'Height (MSL)', value: pos.height),
								const SizedBox(height: 16),
								Row(
									children: [
										_AccBlock(label: 'hAcc', value: pos.hacc, color: _accColor),
										const SizedBox(width: 24),
										_AccBlock(label: 'vAcc', value: pos.vacc, color: _accColor),
										const Spacer(),
										_AccBlock(label: 'Satellites', value: pos.sats, color: kTextSub),
									],
								),
							],
						),
					),
				],
			),
		);
	}
}

class _CoordBlock extends StatelessWidget {
	const _CoordBlock({required this.label, required this.value, this.large = true});
	final String label;
	final String value;
	final bool large;

	@override
	Widget build(BuildContext context) {
		return Column(
			crossAxisAlignment: CrossAxisAlignment.start,
			children: [
				Text(
					label.toUpperCase(),
					style: GoogleFonts.montserrat(
						fontSize: 10,
						fontWeight: FontWeight.w700,
						letterSpacing: 1.2,
						color: kTextDim,
					),
				),
				const SizedBox(height: 2),
				Text(
					value,
					style: large
							? const TextStyle(
									fontFamily: 'monospace',
									fontSize: 26,
									fontWeight: FontWeight.w700,
									color: Color(0xFFECEFF1),
									letterSpacing: 0.52,
								)
							: GoogleFonts.montserrat(
									fontSize: 16,
									fontWeight: FontWeight.w700,
									color: const Color(0xFF90A4AE)),
				),
			],
		);
	}
}

class _AccBlock extends StatelessWidget {
	const _AccBlock({required this.label, required this.value, required this.color});
	final String label;
	final String value;
	final Color color;

	@override
	Widget build(BuildContext context) {
		return Column(
			crossAxisAlignment: CrossAxisAlignment.start,
			children: [
				Text(
					label.toUpperCase(),
					style: GoogleFonts.montserrat(
						fontSize: 10,
						fontWeight: FontWeight.w700,
						letterSpacing: 1.2,
						color: kTextDim,
					),
				),
				Text(
					value,
					style: TextStyle(
						fontFamily: 'monospace',
						fontSize: 20,
						fontWeight: FontWeight.w700,
						color: color,
					),
				),
			],
		);
	}
}

// ── Right panel ───────────────────────────────────────────────────────────────

class _RightPanel extends StatelessWidget {
	const _RightPanel({
		required this.fix,
		required this.fixState,
		required this.onFixChange,
		required this.points,
		required this.canCapture,
	});

	final ({Color bg, Color border, Color label, Color textColor, String text, String hacc, Color haccColor}) fix;
	final _FixState fixState;
	final ValueChanged<_FixState> onFixChange;
	final List<_CapturedPoint> points;
	final bool canCapture;

	@override
	Widget build(BuildContext context) {
		return Padding(
			padding: const EdgeInsets.fromLTRB(16, 14, 16, 14),
			child: Column(
				crossAxisAlignment: CrossAxisAlignment.stretch,
				children: [
					// Fix quality block
					AnimatedContainer(
						duration: const Duration(milliseconds: 200),
						decoration: BoxDecoration(
							color: fix.bg,
							border: Border.all(color: fix.border),
							borderRadius: BorderRadius.circular(10),
						),
						padding: const EdgeInsets.fromLTRB(20, 16, 20, 16),
						child: Row(
							children: [
								Expanded(
									child: Column(
										crossAxisAlignment: CrossAxisAlignment.start,
										children: [
											Text(
												'FIX QUALITY',
												style: GoogleFonts.montserrat(
													fontSize: 11,
													fontWeight: FontWeight.w700,
													letterSpacing: 1.2,
													color: fix.label,
												),
											),
											const SizedBox(height: 4),
											Text(
												fix.text,
												style: GoogleFonts.montserrat(
													fontSize: 32,
													fontWeight: FontWeight.w700,
													color: fix.textColor,
												),
											),
										],
									),
								),
								Column(
									crossAxisAlignment: CrossAxisAlignment.end,
									children: [
										Text(
											'HORIZONTAL',
											style: GoogleFonts.montserrat(
												fontSize: 11,
												fontWeight: FontWeight.w700,
												letterSpacing: 1.0,
												color: fix.label,
											),
										),
										const SizedBox(height: 4),
										Text(
											fix.hacc,
											style: TextStyle(
												fontFamily: 'monospace',
												fontSize: 28,
												fontWeight: FontWeight.w700,
												color: fix.haccColor,
											),
										),
									],
								),
							],
						),
					),
					const SizedBox(height: 12),

					// Dev fix state toggle
					Row(
						children: [
							_FixBtn(label: '✗ No Fix', state: _FixState.noFix, current: fixState, onTap: () => onFixChange(_FixState.noFix)),
							const SizedBox(width: 4),
							_FixBtn(label: '~ Float', state: _FixState.rtkFloat, current: fixState, onTap: () => onFixChange(_FixState.rtkFloat)),
							const SizedBox(width: 4),
							_FixBtn(label: '✓ RTK Fix', state: _FixState.rtkFix, current: fixState, onTap: () => onFixChange(_FixState.rtkFix)),
						],
					),
					const SizedBox(height: 8),

					// Corrections strip
					Container(
						decoration: BoxDecoration(
							color: kBgStatus,
							border: Border.all(color: const Color(0xFF1E2D3D)),
							borderRadius: BorderRadius.circular(8),
						),
						padding: const EdgeInsets.fromLTRB(16, 12, 16, 12),
						child: Row(
							children: [
								_CorrStat(label: 'Corrections', value: 'RF · GFSK', valueColor: const Color(0xFF64B5F6)),
								const SizedBox(width: 20),
								_CorrStat(label: 'RSSI', value: '−82 dBm'),
								const SizedBox(width: 20),
								_CorrStat(label: 'SNR', value: '9.2 dB'),
								const SizedBox(width: 20),
								_CorrStat(label: 'Frame rate', value: '4.1 fps'),
								const SizedBox(width: 20),
								_CorrStat(label: 'Last frame', value: '0.8 s ago'),
							],
						),
					),
					const SizedBox(height: 8),

					// Captured points
					Expanded(
						child: Column(
							crossAxisAlignment: CrossAxisAlignment.start,
							children: [
								Row(
									children: [
										Text(
											'CAPTURED POINTS',
											style: GoogleFonts.montserrat(
												fontSize: 10,
												fontWeight: FontWeight.w700,
												letterSpacing: 1.0,
												color: kTextVersion,
											),
										),
										const SizedBox(width: 8),
										Text(
											'${points.length}',
											style: GoogleFonts.montserrat(
												fontSize: 10,
												fontWeight: FontWeight.w700,
												color: kBlueButton,
											),
										),
									],
								),
								const SizedBox(height: 6),
								Expanded(
									child: Container(
										decoration: BoxDecoration(
											color: kBgStatus,
											border: Border.all(color: const Color(0xFF1E2D3D)),
											borderRadius: BorderRadius.circular(8),
										),
										child: Column(
											children: [
												_PointsHeader(),
												Expanded(
													child: ListView.builder(
														itemCount: points.length,
														itemBuilder: (_, i) => _PointRow(
																point: points[i],
																alt: i.isOdd),
													),
												),
											],
										),
									),
								),
							],
						),
					),
					const SizedBox(height: 10),

					// Capture button
					SizedBox(
						height: 52,
						child: ElevatedButton(
							onPressed: canCapture ? () {} : null,
							style: ElevatedButton.styleFrom(
								backgroundColor: canCapture
										? const Color(0xFF2E7D32)
										: const Color(0xFF1A1A1A),
								foregroundColor:
										canCapture ? Colors.white : kTextDim,
								disabledBackgroundColor: const Color(0xFF1A1A1A),
								disabledForegroundColor: kTextDim,
								elevation: 0,
								shape: RoundedRectangleBorder(
										borderRadius: BorderRadius.circular(8)),
							),
							child: Text(
								canCapture ? 'Capture Point →' : 'RTK Fix Required',
								style: GoogleFonts.montserrat(
										fontSize: 18, fontWeight: FontWeight.w700),
							),
						),
					),
				],
			),
		);
	}
}

class _FixBtn extends StatelessWidget {
	const _FixBtn({required this.label, required this.state, required this.current, required this.onTap});
	final String label;
	final _FixState state;
	final _FixState current;
	final VoidCallback onTap;

	@override
	Widget build(BuildContext context) {
		final active = state == current;
		return GestureDetector(
			onTap: onTap,
			child: Container(
				padding: const EdgeInsets.symmetric(horizontal: 8, vertical: 3),
				decoration: BoxDecoration(
					color: active ? kBlueButton : const Color(0xFF0D1520),
					borderRadius: BorderRadius.circular(4),
					border: Border.all(
							color: active
									? kBlueButton
									: const Color(0xFF1E2D3D)),
				),
				child: Text(label,
						style: GoogleFonts.montserrat(
							fontSize: 10,
							fontWeight: FontWeight.w700,
							color: active ? Colors.white : kTextMuted,
						)),
			),
		);
	}
}

class _CorrStat extends StatelessWidget {
	const _CorrStat({required this.label, required this.value, this.valueColor});
	final String label;
	final String value;
	final Color? valueColor;

	@override
	Widget build(BuildContext context) {
		return Column(
			crossAxisAlignment: CrossAxisAlignment.start,
			children: [
				Text(label.toUpperCase(),
						style: GoogleFonts.montserrat(
							fontSize: 10,
							fontWeight: FontWeight.w700,
							letterSpacing: 1.0,
							color: const Color(0xFF263238),
						)),
				const SizedBox(height: 3),
				Text(value,
						style: GoogleFonts.montserrat(
							fontSize: 13,
							color: valueColor ?? const Color(0xFF546E7A),
						)),
			],
		);
	}
}

class _PointsHeader extends StatelessWidget {
	@override
	Widget build(BuildContext context) {
		return Container(
			padding: const EdgeInsets.fromLTRB(12, 6, 12, 6),
			decoration: const BoxDecoration(
				border: Border(bottom: BorderSide(color: Color(0xFF1E2D3D))),
			),
			child: Row(
				children: const [
					SizedBox(width: 52, child: _HeaderCell('ID')),
					SizedBox(width: 72, child: _HeaderCell('Label')),
					Expanded(child: _HeaderCell('Latitude')),
					Expanded(child: _HeaderCell('Longitude')),
					SizedBox(width: 80, child: _HeaderCell('Height', right: true)),
					SizedBox(width: 60, child: _HeaderCell('hAcc', right: true)),
					SizedBox(width: 60, child: _HeaderCell('vAcc', right: true)),
				],
			),
		);
	}
}

class _HeaderCell extends StatelessWidget {
	const _HeaderCell(this.text, {this.right = false});
	final String text;
	final bool right;

	@override
	Widget build(BuildContext context) {
		return Text(
			text.toUpperCase(),
			textAlign: right ? TextAlign.right : TextAlign.left,
			style: GoogleFonts.montserrat(
				fontSize: 9,
				fontWeight: FontWeight.w700,
				letterSpacing: 1.0,
				color: const Color(0xFF263238),
			),
		);
	}
}

class _PointRow extends StatelessWidget {
	const _PointRow({required this.point, required this.alt});
	final _CapturedPoint point;
	final bool alt;

	@override
	Widget build(BuildContext context) {
		return Container(
			color: alt ? const Color(0xFF111820) : Colors.transparent,
			padding: const EdgeInsets.fromLTRB(12, 8, 12, 8),
			child: Row(
				children: [
					SizedBox(
						width: 52,
						child: _MonoCell(point.id, color: kTextDim),
					),
					SizedBox(
						width: 72,
						child: _MonoCell(point.label,
								color: point.labelHighlight
										? const Color(0xFF64B5F6)
										: const Color(0xFF546E7A)),
					),
					Expanded(child: _MonoCell(point.lat)),
					Expanded(child: _MonoCell(point.lon)),
					SizedBox(
						width: 80,
						child: _MonoCell(point.height, right: true),
					),
					SizedBox(
						width: 60,
						child: _MonoCell('${point.hacc} mm',
								color: const Color(0xFF66BB6A), right: true),
					),
					SizedBox(
						width: 60,
						child: _MonoCell('${point.vacc} mm',
								color: const Color(0xFF66BB6A), right: true),
					),
				],
			),
		);
	}
}

class _MonoCell extends StatelessWidget {
	const _MonoCell(this.text, {this.color, this.right = false});
	final String text;
	final Color? color;
	final bool right;

	@override
	Widget build(BuildContext context) {
		return Text(
			text,
			textAlign: right ? TextAlign.right : TextAlign.left,
			style: TextStyle(
				fontFamily: 'monospace',
				fontSize: 12,
				color: color ?? const Color(0xFF546E7A),
			),
		);
	}
}

class _CapturedPoint {
	const _CapturedPoint({
		required this.id,
		required this.label,
		required this.lat,
		required this.lon,
		required this.height,
		required this.hacc,
		required this.vacc,
		this.labelHighlight = false,
	});

	final String id;
	final String label;
	final String lat;
	final String lon;
	final String height;
	final String hacc;
	final String vacc;
	final bool labelHighlight;
}
