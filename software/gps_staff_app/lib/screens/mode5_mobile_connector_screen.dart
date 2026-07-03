import 'package:flutter/material.dart';
import 'package:google_fonts/google_fonts.dart';
import '../theme.dart';
import '../widgets/status_bar.dart';

enum _ConnState { disconnected, connecting, connected }

class Mode5MobileConnectorScreen extends StatefulWidget {
	const Mode5MobileConnectorScreen({super.key});

	@override
	State<Mode5MobileConnectorScreen> createState() =>
			_Mode5MobileConnectorScreenState();
}

class _Mode5MobileConnectorScreenState
		extends State<Mode5MobileConnectorScreen> {
	_ConnState _state = _ConnState.disconnected;
	String _remoteHost = 'myhome.example.com';
	String _remotePort = '2101';

	@override
	Widget build(BuildContext context) {
		return Scaffold(
			backgroundColor: kLightBg,
      resizeToAvoidBottomInset: false,
			body: Column(
				crossAxisAlignment: CrossAxisAlignment.stretch,
				children: [
					const StatusBar(),
					_Header(onBack: () => Navigator.pop(context)),
					Expanded(
						child: Row(
							crossAxisAlignment: CrossAxisAlignment.stretch,
							children: [
								_LeftPanel(connState: _state),
								Expanded(
									child: _RightPanel(
										remoteHost: _remoteHost,
										remotePort: _remotePort,
										connState: _state,
										onStateChange: (s) =>
												setState(() => _state = s),
										onHostEdit: () => _editField(
											context,
											label: 'Remote Hostname or IP',
											initial: _remoteHost,
											numeric: false,
											onDone: (v) =>
													setState(() => _remoteHost = v),
										),
										onPortEdit: () => _editField(
											context,
											label: 'Remote Port',
											initial: _remotePort,
											numeric: true,
											onDone: (v) =>
													setState(() => _remotePort = v),
										),
									),
								),
							],
						),
					),
				],
			),
		);
	}

	Future<void> _editField(
		BuildContext context, {
		required String label,
		required String initial,
		required bool numeric,
		required ValueChanged<String> onDone,
	}) async {
		final controller = TextEditingController(text: initial);
		final result = await showDialog<String>(
			context: context,
			builder: (ctx) => AlertDialog(
				backgroundColor: kLightBg,
				title: Text(label,
						style: GoogleFonts.montserrat(
								fontWeight: FontWeight.w700, color: kLightText)),
				content: TextField(
					controller: controller,
					autofocus: true,
					keyboardType: numeric ? TextInputType.number : TextInputType.url,
					style: GoogleFonts.montserrat(fontSize: 16, color: kLightText),
				),
				actions: [
					TextButton(
						onPressed: () => Navigator.pop(ctx),
						child: Text('Cancel',
								style: GoogleFonts.montserrat(color: kLightTextMuted)),
					),
					ElevatedButton(
						onPressed: () => Navigator.pop(ctx, controller.text),
						style: ElevatedButton.styleFrom(
								backgroundColor: kBlueButton,
								foregroundColor: Colors.white),
						child: Text('Confirm',
								style: GoogleFonts.montserrat(
										fontWeight: FontWeight.w700)),
					),
				],
			),
		);
		if (result != null && result.isNotEmpty) onDone(result);
	}
}

// ── Header ────────────────────────────────────────────────────────────────────

class _Header extends StatelessWidget {
	const _Header({required this.onBack});
	final VoidCallback onBack;

	@override
	Widget build(BuildContext context) {
		return Container(
			height: 52,
			color: kLightHeaderBg,
			padding: const EdgeInsets.symmetric(horizontal: 16),
			child: Row(
				children: [
					GestureDetector(
						onTap: onBack,
						behavior: HitTestBehavior.opaque,
						child: Container(
							height: 52,
							padding: const EdgeInsets.symmetric(horizontal: 16),
							alignment: Alignment.center,
							child: Text('← Back',
									style: GoogleFonts.montserrat(
											fontSize: 15,
											fontWeight: FontWeight.w600,
											color: Colors.white,
											height: 1)),
						),
					),
					Expanded(
						child: Center(
							child: Text('Base Config',
									style: GoogleFonts.montserrat(
											fontSize: 20,
											fontWeight: FontWeight.w700,
											color: Colors.white)),
						),
					),
					Container(
						padding:
								const EdgeInsets.symmetric(horizontal: 10, vertical: 3),
						decoration: BoxDecoration(
							color: const Color(0xFF1A0A2E),
							border: Border.all(color: const Color(0xFF4A148C)),
							borderRadius: BorderRadius.circular(4),
						),
						child: Text(
							'Mode 5 · Mobile',
							style: GoogleFonts.montserrat(
								fontSize: 11,
								fontWeight: FontWeight.w700,
								color: const Color(0xFFCE93D8),
							),
						),
					),
				],
			),
		);
	}
}

// ── Left panel ────────────────────────────────────────────────────────────────

class _LeftPanel extends StatelessWidget {
	const _LeftPanel({required this.connState});
	final _ConnState connState;

	@override
	Widget build(BuildContext context) {
		final isConnecting = connState == _ConnState.connecting;
		final isConnected = connState == _ConnState.connected;
		final isDisconnected = connState == _ConnState.disconnected;

		return Container(
			width: 420,
			decoration: const BoxDecoration(
				color: Color(0xFFF8F9FA),
				border: Border(right: BorderSide(color: Color(0xFFE0E0E0))),
			),
			padding: const EdgeInsets.all(20),
			child: Column(
				crossAxisAlignment: CrossAxisAlignment.start,
				children: [
					_SectionLabel('TCP Connection'),
					const SizedBox(height: 10),
					Container(
						decoration: BoxDecoration(
							color: Colors.white,
							border: Border.all(color: const Color(0xFFE0E0E0)),
							borderRadius: BorderRadius.circular(8),
						),
						padding: const EdgeInsets.all(16),
						child: Column(
							crossAxisAlignment: CrossAxisAlignment.start,
							children: [
								Row(
									children: [
										if (isConnecting)
											SizedBox(
												width: 14,
												height: 14,
												child: CircularProgressIndicator(
													strokeWidth: 2,
													color: kBlueButton,
												),
											)
										else
											Container(
												width: 14,
												height: 14,
												decoration: BoxDecoration(
													color: isConnected
															? const Color(0xFFE8F5E9)
															: const Color(0xFFECEFF1),
													shape: BoxShape.circle,
													border: Border.all(
														color: isConnected
																? const Color(0xFF43A047)
																: const Color(0xFF90A4AE),
														width: 2,
													),
												),
											),
										const SizedBox(width: 12),
										Expanded(
											child: Column(
												crossAxisAlignment:
														CrossAxisAlignment.start,
												children: [
													Text(
														isConnecting
																? 'Connecting…'
																: isConnected
																		? 'Connected'
																		: 'Not connected',
														style: GoogleFonts.montserrat(
															fontSize: 14,
															fontWeight: FontWeight.w700,
															color: isConnecting
																	? kBlueButton
																	: isConnected
																			? const Color(
																					0xFF2E7D32)
																			: const Color(
																					0xFF90A4AE),
														),
													),
													Text(
														isConnecting
																? 'Reaching myhome.example.com:2101'
																: isConnected
																		? 'TCP link to remote Rover active'
																		: 'Enter remote address and tap Connect',
														style: GoogleFonts.montserrat(
															fontSize: 11,
															color:
																	const Color(0xFFBBBBBB),
														),
													),
												],
											),
										),
									],
								),
								if (isConnected) ...[
									const SizedBox(height: 12),
									_StatRow('Remote address',
											'myhome.example.com:2101'),
									_StatRow('RTCM received', '0 bytes'),
									_StatRow('Connected for', '00:00:12'),
								],
							],
						),
					),
					const SizedBox(height: 18),
					_SectionLabel('Base F9P'),
					const SizedBox(height: 10),
					Container(
						decoration: BoxDecoration(
							color: Colors.white,
							border: Border.all(color: const Color(0xFFE0E0E0)),
							borderRadius: BorderRadius.circular(8),
						),
						padding: const EdgeInsets.fromLTRB(14, 14, 14, 10),
						child: Column(
							crossAxisAlignment: CrossAxisAlignment.start,
							children: [
								_StatRow(
									'Survey-in',
									isConnected ? 'Ready to start' : 'Not started',
									valueColor: isConnected
											? const Color(0xFFFF8F00)
											: null,
									valueDim: !isConnected,
								),
								_StatRow(
									'RTCM output',
									isConnected ? 'Not streaming yet' : '—',
									valueDim: true,
								),
							],
						),
					),
					const SizedBox(height: 8),
					Text(
						'Start survey-in from Base Config once TCP link is established',
						style: GoogleFonts.montserrat(
								fontSize: 11, color: const Color(0xFFAAAAAA)),
					),
				],
			),
		);
	}
}

class _SectionLabel extends StatelessWidget {
	const _SectionLabel(this.text);
	final String text;

	@override
	Widget build(BuildContext context) {
		return Text(
			text.toUpperCase(),
			style: GoogleFonts.montserrat(
				fontSize: 11,
				fontWeight: FontWeight.w700,
				letterSpacing: 1.0,
				color: const Color(0xFF999999),
			),
		);
	}
}

class _StatRow extends StatelessWidget {
	const _StatRow(this.label, this.value,
			{this.valueDim = false, this.valueColor});
	final String label;
	final String value;
	final bool valueDim;
	final Color? valueColor;

	@override
	Widget build(BuildContext context) {
		return Padding(
			padding: const EdgeInsets.symmetric(vertical: 5),
			child: Row(
				mainAxisAlignment: MainAxisAlignment.spaceBetween,
				children: [
					Text(label,
							style: GoogleFonts.montserrat(
									fontSize: 13,
									color: const Color(0xFF888888))),
					Flexible(
						child: Text(
							value,
							textAlign: TextAlign.right,
							style: GoogleFonts.montserrat(
								fontSize: 13,
								fontWeight: FontWeight.w700,
								color: valueColor ??
										(valueDim
												? const Color(0xFFCCCCCC)
												: kLightText),
							),
						),
					),
				],
			),
		);
	}
}

// ── Right panel ───────────────────────────────────────────────────────────────

class _RightPanel extends StatelessWidget {
	const _RightPanel({
		required this.remoteHost,
		required this.remotePort,
		required this.connState,
		required this.onStateChange,
		required this.onHostEdit,
		required this.onPortEdit,
	});

	final String remoteHost;
	final String remotePort;
	final _ConnState connState;
	final ValueChanged<_ConnState> onStateChange;
	final VoidCallback onHostEdit;
	final VoidCallback onPortEdit;

	bool get _locked => connState != _ConnState.disconnected;

	@override
	Widget build(BuildContext context) {
		return Padding(
			padding: const EdgeInsets.fromLTRB(24, 20, 24, 20),
			child: Column(
				crossAxisAlignment: CrossAxisAlignment.start,
				children: [
					_SectionLabel('Remote Address'),
					const SizedBox(height: 10),
					_EditableField(
						sublabel: 'Router hostname or IP',
						value: remoteHost,
						locked: _locked,
						onTap: onHostEdit,
					),
					const SizedBox(height: 16),

					_SectionLabel('Remote Port'),
					const SizedBox(height: 10),
					_EditableField(
						sublabel: 'Must match listener port and router forward',
						value: remotePort,
						locked: _locked,
						onTap: onPortEdit,
					),
					const SizedBox(height: 18),

					_SectionLabel('System Mode'),
					const SizedBox(height: 10),
					Container(
						decoration: BoxDecoration(
							color: const Color(0xFFF5F5F5),
							border: Border.all(color: const Color(0xFFE0E0E0)),
							borderRadius: BorderRadius.circular(6),
						),
						padding: const EdgeInsets.symmetric(
								horizontal: 14, vertical: 10),
						child: Row(
							mainAxisAlignment:
									MainAxisAlignment.spaceBetween,
							children: [
								Column(
									crossAxisAlignment:
											CrossAxisAlignment.start,
									children: [
										Text('Mode 5 — Direct TCP',
												style: GoogleFonts.montserrat(
														fontSize: 13,
														color: const Color(0xFF888888))),
										Text(
											'This device: mobile connector · Remote: Rover',
											style: GoogleFonts.montserrat(
												fontSize: 11,
												color: const Color(0xFFBBBBBB),
											),
										),
									],
								),
								Text('Change ›',
										style: GoogleFonts.montserrat(
												fontSize: 11,
												color: const Color(0xFFBBBBBB))),
							],
						),
					),

					const Spacer(),

					// Dev state picker
					Wrap(
						spacing: 4,
						children: [
							_StateBtn(
								label: '○ Disconnected',
								active: connState == _ConnState.disconnected,
								onTap: () =>
										onStateChange(_ConnState.disconnected),
							),
							_StateBtn(
								label: '◌ Connecting',
								active: connState == _ConnState.connecting,
								onTap: () =>
										onStateChange(_ConnState.connecting),
							),
							_StateBtn(
								label: '● Connected',
								active: connState == _ConnState.connected,
								onTap: () =>
										onStateChange(_ConnState.connected),
							),
						],
					),
					const SizedBox(height: 12),

					SizedBox(
						width: double.infinity,
						height: 48,
						child: ElevatedButton(
							onPressed: connState == _ConnState.disconnected
									? () => onStateChange(_ConnState.connecting)
									: connState == _ConnState.connecting
											? () => onStateChange(_ConnState.disconnected)
											: () => onStateChange(_ConnState.disconnected),
							style: ElevatedButton.styleFrom(
								backgroundColor: connState == _ConnState.disconnected
										? kBlueButton
										: connState == _ConnState.connecting
												? const Color(0xFF455A64)
												: const Color(0xFFB71C1C),
								foregroundColor: Colors.white,
								elevation: 0,
								shape: RoundedRectangleBorder(
										borderRadius: BorderRadius.circular(8)),
							),
							child: Text(
								connState == _ConnState.disconnected
										? 'Connect to Remote Rover →'
										: connState == _ConnState.connecting
												? 'Cancel'
												: 'Disconnect',
								style: GoogleFonts.montserrat(
										fontSize: 15, fontWeight: FontWeight.w700),
							),
						),
					),
				],
			),
		);
	}
}

class _EditableField extends StatelessWidget {
	const _EditableField({
		required this.sublabel,
		required this.value,
		required this.locked,
		required this.onTap,
	});

	final String sublabel;
	final String value;
	final bool locked;
	final VoidCallback onTap;

	@override
	Widget build(BuildContext context) {
		return GestureDetector(
			onTap: locked ? null : onTap,
			child: Container(
				decoration: BoxDecoration(
					color: locked ? const Color(0xFFF5F5F5) : Colors.white,
					border: Border.all(
							color: const Color(0xFFE0E0E0), width: 2),
					borderRadius: BorderRadius.circular(6),
				),
				padding: const EdgeInsets.symmetric(
						horizontal: 14, vertical: 10),
				child: Row(
					children: [
						Expanded(
							child: Column(
								crossAxisAlignment: CrossAxisAlignment.start,
								children: [
									Text(sublabel,
											style: GoogleFonts.montserrat(
													fontSize: 12,
													color: const Color(0xFFAAAAAA))),
									const SizedBox(height: 3),
									Text(
										value,
										style: GoogleFonts.montserrat(
											fontSize: 16,
											fontWeight: FontWeight.w700,
											color: locked
													? const Color(0xFF888888)
													: kLightText,
										),
										overflow: TextOverflow.ellipsis,
									),
								],
							),
						),
						if (!locked)
							Text('✎',
									style: TextStyle(
											fontSize: 16,
											color: const Color(0xFFBBBBBB))),
					],
				),
			),
		);
	}
}

class _StateBtn extends StatelessWidget {
	const _StateBtn(
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
					color: active ? kBlueButton : const Color(0xFFF5F5F5),
					borderRadius: BorderRadius.circular(4),
					border: Border.all(
							color: active
									? kBlueButton
									: const Color(0xFFE0E0E0)),
				),
				child: Text(label,
						style: GoogleFonts.montserrat(
							fontSize: 11,
							fontWeight: FontWeight.w700,
							color:
									active ? Colors.white : kLightTextMuted,
						)),
			),
		);
	}
}
