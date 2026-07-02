import 'package:flutter/material.dart';
import 'package:google_fonts/google_fonts.dart';
import '../theme.dart';
import '../widgets/status_bar.dart';

class UsbStorageActiveScreen extends StatefulWidget {
	const UsbStorageActiveScreen({super.key});

	@override
	State<UsbStorageActiveScreen> createState() =>
			_UsbStorageActiveScreenState();
}

class _UsbStorageActiveScreenState extends State<UsbStorageActiveScreen> {
	bool _ejected = false;

	@override
	Widget build(BuildContext context) {
		return Scaffold(
			backgroundColor: kBgHome,
      resizeToAvoidBottomInset: false,
			body: Column(
				crossAxisAlignment: CrossAxisAlignment.stretch,
				children: [
					_TopBar(),
					_Header(),
					Expanded(
						child: Row(
							crossAxisAlignment: CrossAxisAlignment.stretch,
							children: [
								_LeftPanel(),
								Expanded(
									child: _RightPanel(
										ejected: _ejected,
										onToggle: () =>
												setState(() => _ejected = !_ejected),
										onDone: () => Navigator.pop(context),
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

// ── Top bar (dual battery + clock) ───────────────────────────────────────────

class _TopBar extends StatelessWidget {
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
							const Text('▐████▌ 91%',
									style: TextStyle(
											fontFamily: 'monospace',
											fontSize: 11,
											color: Color(0xFF66BB6A))),
							const SizedBox(width: 16),
							Text('Rover ',
									style: GoogleFonts.montserrat(
											fontSize: 11, color: kTextMuted)),
							const Text('▐███░▌ 72%',
									style: TextStyle(
											fontFamily: 'monospace',
											fontSize: 11,
											color: Color(0xFF66BB6A))),
						],
					),
					Row(
						children: [
							const Text('2026/07/01 14:23:47',
									style: TextStyle(
											fontFamily: 'monospace',
											fontSize: 11,
											color: Color(0xFF37474F))),
							const SizedBox(width: 14),
							const Text('▐███░▌ 78%',
									style: TextStyle(
											fontFamily: 'monospace',
											fontSize: 11,
											color: Color(0xFF66BB6A))),
						],
					),
				],
			),
		);
	}
}

class _Header extends StatelessWidget {
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
					Expanded(
						child: Text(
							'USB File Transfer',
							style: GoogleFonts.montserrat(
								fontSize: 16,
								fontWeight: FontWeight.w700,
								color: const Color(0xFF78909C),
							),
						),
					),
					Text(
						'ALL OTHER FUNCTIONS SUSPENDED',
						style: GoogleFonts.montserrat(
							fontSize: 11,
							fontWeight: FontWeight.w700,
							letterSpacing: 1.0,
							color: kTextDim,
						),
					),
				],
			),
		);
	}
}

// ── Left panel ────────────────────────────────────────────────────────────────

class _LeftPanel extends StatelessWidget {
	@override
	Widget build(BuildContext context) {
		return Container(
			width: 280,
			decoration: const BoxDecoration(
				border: Border(right: BorderSide(color: Color(0xFF1E2D3D))),
			),
			padding: const EdgeInsets.fromLTRB(20, 24, 20, 24),
			child: Column(
				crossAxisAlignment: CrossAxisAlignment.start,
				children: [
					Text(
						'SD CARD CONTENTS',
						style: GoogleFonts.montserrat(
							fontSize: 10,
							fontWeight: FontWeight.w700,
							letterSpacing: 1.2,
							color: const Color(0xFF263238),
						),
					),
					const SizedBox(height: 10),
					_CardStat(label: 'Sessions', value: '4'),
					const SizedBox(height: 12),
					_CardStat(label: 'Total size', value: '18 KB'),
					const SizedBox(height: 12),
					Column(
						crossAxisAlignment: CrossAxisAlignment.start,
						children: [
							Text(
								'LAST SESSION',
								style: GoogleFonts.montserrat(
									fontSize: 10,
									fontWeight: FontWeight.w700,
									letterSpacing: 1.0,
									color: const Color(0xFF263238),
								),
							),
							const SizedBox(height: 3),
							const Text(
								'survey_004.log',
								style: TextStyle(
									fontFamily: 'monospace',
									fontSize: 13,
									fontWeight: FontWeight.w700,
									color: Color(0xFF37474F),
								),
							),
						],
					),
					const Spacer(),
					Text(
						'These figures were read before the SD card was handed to USB. '
						'The card may have changed while connected.',
						style: GoogleFonts.montserrat(
							fontSize: 11,
							color: const Color(0xFF1E2D3D),
							height: 1.7,
						),
					),
				],
			),
		);
	}
}

class _CardStat extends StatelessWidget {
	const _CardStat({required this.label, required this.value});
	final String label;
	final String value;

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
						letterSpacing: 1.0,
						color: const Color(0xFF263238),
					),
				),
				const SizedBox(height: 3),
				Text(
					value,
					style: const TextStyle(
						fontFamily: 'monospace',
						fontSize: 24,
						fontWeight: FontWeight.w700,
						color: Color(0xFF546E7A),
					),
				),
			],
		);
	}
}

// ── Right panel ───────────────────────────────────────────────────────────────

class _RightPanel extends StatelessWidget {
	const _RightPanel({
		required this.ejected,
		required this.onToggle,
		required this.onDone,
	});

	final bool ejected;
	final VoidCallback onToggle;
	final VoidCallback onDone;

	@override
	Widget build(BuildContext context) {
		return Column(
			mainAxisAlignment: MainAxisAlignment.center,
			children: [
				// USB icon
				Text(
					'⬡',
					style: TextStyle(
						fontSize: 64,
						color: const Color(0xFF1E2D3D),
					),
				),
				const SizedBox(height: 24),

				// Status block
				AnimatedContainer(
					duration: const Duration(milliseconds: 200),
					margin: const EdgeInsets.symmetric(horizontal: 40),
					constraints: const BoxConstraints(minWidth: 400),
					decoration: BoxDecoration(
						color: ejected
								? const Color(0xFF1B5E20)
								: const Color(0xFF1A0F00),
						border: Border.all(
							color: ejected
									? const Color(0xFF2E7D32)
									: const Color(0xFF5E3500),
						),
						borderRadius: BorderRadius.circular(12),
					),
					padding: const EdgeInsets.fromLTRB(40, 20, 40, 20),
					child: Column(
						children: [
							Text(
								'STATUS',
								style: GoogleFonts.montserrat(
									fontSize: 11,
									fontWeight: FontWeight.w700,
									letterSpacing: 1.2,
									color: ejected
											? const Color(0xFF388E3C)
											: const Color(0xFF7E4A00),
								),
							),
							const SizedBox(height: 8),
							Text(
								ejected
										? 'Ejected — safe to disconnect'
										: 'Connected — do not disconnect',
								style: GoogleFonts.montserrat(
									fontSize: 26,
									fontWeight: FontWeight.w700,
									color: ejected
											? const Color(0xFFA5D6A7)
											: const Color(0xFFFFCC80),
								),
								textAlign: TextAlign.center,
							),
							const SizedBox(height: 8),
							Text(
								ejected
										? 'You may unplug the cable or tap Done to return'
										: 'Eject the drive on your PC before disconnecting',
								style: GoogleFonts.montserrat(
									fontSize: 13,
									color: ejected
											? const Color(0xFF2E7D32)
											: const Color(0xFF5E3500),
								),
								textAlign: TextAlign.center,
							),
						],
					),
				),
				const SizedBox(height: 28),

				// Dev toggle
				Row(
					mainAxisAlignment: MainAxisAlignment.center,
					children: [
						_ToggleBtn(
							label: '⚡ Connected',
							active: !ejected,
							onTap: () {
								if (ejected) onToggle();
							},
						),
						const SizedBox(width: 4),
						_ToggleBtn(
							label: '✓ Ejected',
							active: ejected,
							onTap: () {
								if (!ejected) onToggle();
							},
						),
					],
				),
				const SizedBox(height: 16),

				// Action button
				SizedBox(
					width: 320,
					height: 52,
					child: ejected
							? ElevatedButton(
									onPressed: onDone,
									style: ElevatedButton.styleFrom(
										backgroundColor: const Color(0xFF2E7D32),
										foregroundColor: Colors.white,
										elevation: 0,
										shape: RoundedRectangleBorder(
												borderRadius: BorderRadius.circular(8)),
									),
									child: Text('Done — return to normal',
											style: GoogleFonts.montserrat(
													fontSize: 15,
													fontWeight: FontWeight.w700)),
								)
							: OutlinedButton(
									onPressed: onDone,
									style: OutlinedButton.styleFrom(
										foregroundColor: const Color(0xFF7E4A00),
										side: const BorderSide(
												color: Color(0xFF5E3500)),
										shape: RoundedRectangleBorder(
												borderRadius: BorderRadius.circular(8)),
									),
									child: Text('Disconnect anyway…',
											style: GoogleFonts.montserrat(
												fontSize: 15,
												fontWeight: FontWeight.w700,
											)),
								),
				),
			],
		);
	}
}

class _ToggleBtn extends StatelessWidget {
	const _ToggleBtn(
			{required this.label,
			required this.active,
			required this.onTap});
	final String label;
	final bool active;
	final VoidCallback onTap;

	@override
	Widget build(BuildContext context) {
		return GestureDetector(
			onTap: onTap,
			child: Container(
				padding: const EdgeInsets.symmetric(
						horizontal: 10, vertical: 4),
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
							fontSize: 11,
							fontWeight: FontWeight.w700,
							color: active ? Colors.white : kTextMuted,
						)),
			),
		);
	}
}
