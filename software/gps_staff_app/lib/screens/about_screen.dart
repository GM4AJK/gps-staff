import 'package:flutter/material.dart';
import 'package:google_fonts/google_fonts.dart';
import '../theme.dart';
import '../widgets/status_bar.dart';
import 'links_screen.dart';
import 'usb_storage_active_screen.dart';

class AboutScreen extends StatelessWidget {
	const AboutScreen({super.key});

	@override
	Widget build(BuildContext context) {
		return Scaffold(
			backgroundColor: kBgHome,
      resizeToAvoidBottomInset: false,
			body: Column(
				crossAxisAlignment: CrossAxisAlignment.stretch,
				children: [
					const StatusBar(),
					DarkHeaderBar(
						title: 'About',
						onBack: () => Navigator.pop(context),
					),
					const Expanded(child: _Body()),
				],
			),
		);
	}
}

class DarkHeaderBar extends StatelessWidget {
	const DarkHeaderBar({required this.title, required this.onBack});
	final String title;
	final VoidCallback onBack;

	@override
	Widget build(BuildContext context) {
		return Container(
			height: 52,
			color: kBgStatus,
			child: Stack(
				alignment: Alignment.center,
				children: [
					Text(title,
						style: GoogleFonts.montserrat(
							fontSize: 18,
							fontWeight: FontWeight.w700,
							color: kTextPrimary)),
					Positioned(
						left: 0,
						top: 0,
						bottom: 0,
						child: GestureDetector(
							onTap: onBack,
							behavior: HitTestBehavior.opaque,
							child: Container(
								padding: const EdgeInsets.symmetric(horizontal: 20),
								alignment: Alignment.center,
								child: Text('← Back',
									style: GoogleFonts.montserrat(
										fontSize: 15,
										fontWeight: FontWeight.w600,
										color: kTextMuted,
										height: 1)),
							),
						),
					),
				],
			),
		);
	}
}

class _Body extends StatelessWidget {
	const _Body();

	@override
	Widget build(BuildContext context) {
		return Container(
			color: kBgHome,
			padding: const EdgeInsets.fromLTRB(36, 24, 36, 24),
			child: Column(
				crossAxisAlignment: CrossAxisAlignment.start,
				children: [
					// Identity row
					Row(
						crossAxisAlignment: CrossAxisAlignment.start,
						children: [
							CustomPaint(
								size: const Size(120, 80),
								painter: _StaffLogoPainter(),
							),
							const SizedBox(width: 18),
							Padding(
								padding: const EdgeInsets.only(top: 6),
								child: Column(
									crossAxisAlignment: CrossAxisAlignment.start,
									children: [
										Text(
											'GPS Staff',
											style: GoogleFonts.montserrat(
												fontSize: 32,
												fontWeight: FontWeight.w700,
												color: kTextPrimary,
												letterSpacing: 0.64,
												height: 1,
											),
										),
										const SizedBox(height: 5),
										Text(
											'RTK SURVEY INSTRUMENT',
											style: GoogleFonts.montserrat(
												fontSize: 12,
												color: kTextMuted,
												letterSpacing: 1.2,
											),
										),
									],
								),
							),
						],
					),
					const SizedBox(height: 20),

					// Version grid
					Container(
						decoration: BoxDecoration(
							color: Colors.white.withValues(alpha: 0.03),
							borderRadius: BorderRadius.circular(8),
							border: Border.all(
									color: Colors.white.withValues(alpha: 0.06)),
						),
						padding: const EdgeInsets.fromLTRB(16, 12, 16, 12),
						child: Row(
							children: const [
								Expanded(
										child: _VersionCell(
												label: 'Tablet firmware', value: 'v0.1.0')),
								Expanded(
										child: _VersionCell(label: 'PCB', value: 'v1.0')),
								Expanded(
										child: _VersionCell(
												label: 'GNSS',
												value: 'u-blox ZED-F9P-05B')),
								Expanded(
										child: _VersionCell(
												label: 'Radio',
												value: 'Semtech SX1262 · GFSK')),
							],
						),
					),
					const SizedBox(height: 18),

					_Divider(),
					const SizedBox(height: 16),

					// Wizard note
					Text(
						'✦  The GPS Staff Wizard',
						style: GoogleFonts.montserrat(
							fontSize: 11,
							fontWeight: FontWeight.w700,
							color: const Color(0xFF4527A0),
							letterSpacing: 1.0,
						),
					),
					const SizedBox(height: 8),
					Text(
						'Original mascot of the project. Retired from the home screen in favour\n'
						'of something more boardroom-appropriate. He refused to leave entirely.\n'
						'His staff still points skyward. So does ours.',
						style: GoogleFonts.montserrat(
							fontSize: 13,
							color: kTextSub,
							height: 1.6,
						),
					),
					const SizedBox(height: 14),

					_Divider(),
					const SizedBox(height: 14),

					// Built with
					Text(
						'BUILT WITH',
						style: GoogleFonts.montserrat(
							fontSize: 10,
							fontWeight: FontWeight.w700,
							color: kTextMuted,
							letterSpacing: 1.0,
						),
					),
					const SizedBox(height: 6),
					Text(
						'ESP-IDF · LVGL 9.x · FreeRTOS · u-blox UBX · STM32 HAL',
						style: GoogleFonts.montserrat(
								fontSize: 12, color: const Color(0xFF607D8B)),
					),
					const SizedBox(height: 2),
					Text(
						'FatFS · SX1262 GFSK · OS Net NTRIP · ETSI EN 300 220',
						style: GoogleFonts.montserrat(
								fontSize: 12, color: const Color(0xFF607D8B)),
					),

					const Spacer(),

					// Bottom buttons
					Row(
						children: [
							_BottomButton(
								label: 'Links →',
								onTap: () => Navigator.push(context,
										MaterialPageRoute(
												builder: (_) => const LinksScreen())),
							),
							const SizedBox(width: 10),
							_BottomButton(
								label: 'Transfer Files (USB) →',
								onTap: () => Navigator.push(context,
										MaterialPageRoute(
												builder: (_) => const UsbStorageActiveScreen())),
							),
						],
					),
					const SizedBox(height: 12),

					// Footer
					Row(
						mainAxisAlignment: MainAxisAlignment.spaceBetween,
						crossAxisAlignment: CrossAxisAlignment.end,
						children: [
							Text(
								'GM4AJK · 2026',
								style: GoogleFonts.montserrat(
										fontSize: 11, color: const Color(0xFF455A64)),
							),
							Text(
								'GPS STAFF · PCB v1.0 · fw 0.1.0',
								style: GoogleFonts.montserrat(
									fontSize: 10,
									color: const Color(0xFF37474F),
									letterSpacing: 0.6,
								),
							),
						],
					),
				],
			),
		);
	}
}

class _BottomButton extends StatelessWidget {
	const _BottomButton({required this.label, required this.onTap});
	final String label;
	final VoidCallback onTap;

	@override
	Widget build(BuildContext context) {
		return GestureDetector(
			onTap: onTap,
			behavior: HitTestBehavior.opaque,
			child: Container(
				padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 10),
				decoration: BoxDecoration(
					border: Border.all(color: Colors.white.withValues(alpha: 0.08)),
					borderRadius: BorderRadius.circular(6),
				),
				child: Text(
					label,
					style: GoogleFonts.montserrat(
						fontSize: 13,
						fontWeight: FontWeight.w700,
						color: kTextMuted,
					),
				),
			),
		);
	}
}

class _Divider extends StatelessWidget {
	@override
	Widget build(BuildContext context) {
		return Container(
			height: 1,
			color: Colors.white.withValues(alpha: 0.06),
		);
	}
}

class _VersionCell extends StatelessWidget {
	const _VersionCell({required this.label, required this.value});
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
						color: kTextMuted,
						letterSpacing: 0.8,
					),
				),
				const SizedBox(height: 2),
				Text(
					value,
					style: GoogleFonts.montserrat(
						fontSize: 14,
						fontWeight: FontWeight.w700,
						color: kTextSub,
					),
				),
			],
		);
	}
}

class _StaffLogoPainter extends CustomPainter {
	@override
	void paint(Canvas canvas, Size size) {
		Paint arcPaint(Color color) => Paint()
			..color = color
			..style = PaintingStyle.stroke
			..strokeWidth = 4
			..strokeCap = StrokeCap.round;

		final cx = size.width / 2;
		final r1 = size.width * 0.47;
		final r2 = size.width * 0.33;
		final r3 = size.width * 0.18;
		final cy = size.height * 0.6;

		canvas.drawArc(
				Rect.fromCenter(center: Offset(cx, cy), width: r1 * 2, height: r1 * 2),
				3.14159, 3.14159, false, arcPaint(const Color(0x592196F3)));
		canvas.drawArc(
				Rect.fromCenter(center: Offset(cx, cy), width: r2 * 2, height: r2 * 2),
				3.14159, 3.14159, false, arcPaint(const Color(0x992196F3)));
		canvas.drawArc(
				Rect.fromCenter(center: Offset(cx, cy), width: r3 * 2, height: r3 * 2),
				3.14159, 3.14159, false, arcPaint(const Color(0xFF2196F3)));

		canvas.drawCircle(Offset(cx, cy), 6,
				Paint()..color = const Color(0xFF2196F3));

		canvas.drawLine(
			Offset(cx, cy + 6),
			Offset(cx, size.height * 0.92),
			Paint()
				..color = const Color(0xFF546E7A)
				..strokeWidth = 4
				..strokeCap = StrokeCap.round,
		);
	}

	@override
	bool shouldRepaint(covariant CustomPainter oldDelegate) => false;
}
