import 'package:flutter/material.dart';
import 'screens/home_screen.dart';

void main() {
  runApp(const GpsStaffApp());
}

class GpsStaffApp extends StatelessWidget {
  const GpsStaffApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'GPS Staff',
      debugShowCheckedModeBanner: false,
      theme: ThemeData(
        colorScheme: ColorScheme.fromSeed(
          seedColor: const Color(0xFF2196F3),
          brightness: Brightness.dark,
        ),
      ),
      home: const HomeScreen(),
    );
  }
}
